#pragma once

#include "Cluster/DisplayClusterClusterEvent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Delegates/IntegerSequence.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/NoExportTypes.h"

#include "ClusterSyncedMethodCaller.generated.h"

typedef TMap<FString, FString> FStringMap;
DECLARE_DELEGATE_OneParam(FSyncMethod, FStringMap)
	// typedef FSyncMethod::FDelegate FSyncMethodDelegate;

	UCLASS() class DISPLAYCLUSTEREXTENSIONS_API UClusterSyncedMethodCaller : public UObject
{
	GENERATED_BODY()

public:
	// RegisterListener should be called in BeginPlay()
	void RegisterListener();

	// DeregisterListener() should be called in EndPlay()
	void DeregisterListener();

	// This will call a with UniqueIdentifier registered method in a synced manner with given parameters, also works in non-cluster
	// mode. Notice that in the cluster case this call is synced between all nodes, but async to the call of this method!
	void CallMethodSynced(FString UniqueIdentifier, TMap<FString, FString> Parameters);

	// Register a method to be called in a synced manner, UniqueIdentifier should be unique to any call of this method of any
	// object!
	void RegisterSyncedMethod(FString UniqueIdentifier, FSyncMethod MethodToCall);

	// Calls a previosuly registered delegate with automatic type conversion.
	// Only compatible with delegates registered with RegisterAutoTypedDelegate().
	template <typename... ArgTypes>
	void CallAutoTypedDelegate(FString UniqueIdentifier, ArgTypes&&... Arguments);

	// Registers a delegate with automatic type conversion.
	// Delegates registered with this function should only be called via CallAutoTypedDelegate().
	template <typename RetType, typename... ArgTypes>
	void RegisterAutoTypedDelegate(FString UniqueIdentifier, const TBaseDelegate<RetType, ArgTypes...>& Delegate);

private:
	FOnClusterEventListener ClusterEventListenerDelegate;
	virtual void HandleClusterEvent(const FDisplayClusterClusterEvent& Event);

	TMap<FString, FSyncMethod> SyncedFunctions;
};

// Helper function to put the arguments into a string map. It uses template substitution to choose the correct specialization.
template <typename... Values>
struct FillParameterMapImpl;

// This specialization is chosen when there is at least one argument left to serialize.
template <typename CurrentValueType, typename... RemainingValueTypes>
struct FillParameterMapImpl<CurrentValueType, RemainingValueTypes...>
{
	template <int ArgumentIndex>
	static inline void Invoke(
		TMap<FString, FString>* ParameterMap, const CurrentValueType& CurrentValue, RemainingValueTypes&&... RemainingValues)
	{
		// If this assertion fails: implement the workaround described below!
		static_assert(sizeof(TCHAR) > sizeof(uint8), "TCHAR needs to have extra room!");

		TArray<uint8> SerializedValue;
		// TODO: maybe it is a good idea to guess the amount of bytes the serialized value will have. This would probably reduce the
		// number of reallocations in the serialization process. However, I don't know if this is already taken care of in the
		// FArchive class. Also it is hard to implement correctly, so we have to see whether it is worth it.
		FMemoryWriter writer(SerializedValue);

		// Const cast is necessary here because the "<<" operator is used for reading and writing and thus does not take a const
		// argument. However, it should be fine as, hopefully, the "<<" operator doesn't modify the value in "reading mode".
		auto NonConstCurrentValue = const_cast<CurrentValueType&>(CurrentValue);
		writer << NonConstCurrentValue;

		// We have an array of bytes. Now we need to convert them to a string.
		FString SerializedDataString;
		SerializedDataString.Empty(SerializedValue.Num());	  // Preallocate memory to avoid reallocations.
		for (const uint8 Byte : SerializedValue)
		{
			// This is potentially dangerous:
			// We treat the individual bytes as characters in a string. The character with the value 0 is normally used to mark the
			// end of the string. Because of this, FString will not add zero values to the underlying data array. To avoid this I
			// add 1 to every value. This only works, because the TCHAR is usually more than one byte long. However, this is not
			// guaranteed. Currently I enforce it with a static_assert above. If this is not the case we would need to split each
			// byte into two characters. Another option would be to access the underlying TCHAR array directly. However I don't know
			// if they are transported correctly using the cluster events.
			SerializedDataString += static_cast<TCHAR>(static_cast<TCHAR>(Byte) + 1);
		}

		ParameterMap->Add(FString::FromInt(ArgumentIndex), SerializedDataString);

		// Recursive call for the remaining values.
		FillParameterMapImpl<RemainingValueTypes...>::Invoke<ArgumentIndex + 1>(
			ParameterMap, Forward<RemainingValueTypes>(RemainingValues)...);
	}
};

// This specialization is chosen when there no argument left to serialize.
template <>
struct FillParameterMapImpl<>
{
	template <int ArgumentIndex>
	static inline void Invoke(TMap<FString, FString>*)
	{
		// There is nothing left to do here.
	}
};

// This function fills a string map with the arguments it gets passed. The resulting map will contain an entry for every argument.
// The first argument will have the key "0", the second "1" and so on.
template <typename... ArgTypes>
inline void FillParameterMap(TMap<FString, FString>* ParameterMap, ArgTypes&&... Arguments)
{
	check(ParameterMap != nullptr);
	ParameterMap->Empty(sizeof...(ArgTypes));	 // Preallocate to avoid allocations.
	FillParameterMapImpl<ArgTypes...>::Invoke<0>(ParameterMap, Forward<ArgTypes>(Arguments)...);
}

// Implementation of the method declared above
template <typename... ArgTypes>
void UClusterSyncedMethodCaller::CallAutoTypedDelegate(FString UniqueIdentifier, ArgTypes&&... Arguments)
{
	TMap<FString, FString> ParameterMap;
	// TODO: Add inline construction with a initializer list for performance reasons. I don't know if this is possible. If it is it
	// is possibly be super complicated
	FillParameterMap(&ParameterMap, Forward<ArgTypes>(Arguments)...);

	// We have a regular map now we can pass to the standard method.
	CallMethodSynced(UniqueIdentifier, ParameterMap);
}

// This is a wrapper function to recursively fill the argument tuple. This overload is only used if the index indicating the
// currently handled attribute is less than the number of total attributes. I.e., if the attribute index is valid.
template <int CurrentIndex, typename... ArgTypes>
inline typename TEnableIf<(CurrentIndex < sizeof...(ArgTypes))>::Type FillArgumentTuple(
	TTuple<ArgTypes...>* ArgumentTuple, const TMap<FString, FString>& Parameters)
{
	const FString& SerializedDataString = Parameters[FString::FromInt(CurrentIndex)];
	TArray<uint8> SerializedData;
	// Preallocate to avoid reallocation
	SerializedData.Empty(SerializedDataString.Len());

	// Reconstruct the original bytes. I.e., reversing the addition by one.
	for (const auto Character : SerializedDataString)
	{
		SerializedData.Add(static_cast<uint8>(Character - 1));
	}

	FMemoryReader Reader(SerializedData);
	// Read the "<<" as ">>" operator here. FArchive uses the same for both and decides based on an internal type on what to do. So
	// this statement parses the bytes that were passed into reader and puts the parsed object into the tuple at index CurrentIndex.
	Reader << ArgumentTuple->Get<CurrentIndex>();

	// Recursive call for the remaining attributes.
	FillArgumentTuple<CurrentIndex + 1>(
		Forward<TTuple<ArgTypes...>*>(ArgumentTuple), Forward<const TMap<FString, FString>&>(Parameters));
}

// The overload that is called if we are "passed the end" of attributes.
template <int CurrentIndex, typename... ArgTypes>
inline typename TEnableIf<(CurrentIndex >= sizeof...(ArgTypes))>::Type FillArgumentTuple(
	TTuple<ArgTypes...>* ArgumentTuple, const TMap<FString, FString>& Parameters)
{
}

// Implementation of the method declared above
template <typename RetType, typename... ArgTypes>
void UClusterSyncedMethodCaller::RegisterAutoTypedDelegate(
	FString UniqueIdentifier, const TBaseDelegate<RetType, ArgTypes...>& Delegate)
{
	// Register to the event using a lambda that will do the translation from the string map to the real values.
	RegisterSyncedMethod(UniqueIdentifier, FSyncMethod::CreateLambda([Delegate](TMap<FString, FString> Parameters) {
		// Create a tuple that holds all arguments. This assumes that all argument types are default constructible. However, all
		// types that overload the FArchive "<<" operator probably are.
		TTuple<typename TRemoveCV<typename TRemoveReference<ArgTypes>::Type>::Type...> ArgumentTuple;

		// This call will parse the string map and fill all values in the tuple appropriately.
		FillArgumentTuple<0>(&ArgumentTuple, Parameters);

		// The lambda function is only necessary because delegates do not overload the ()-operator but use the Execute() method
		// instead. So, the lambda acts as a wrapper.
		ArgumentTuple.ApplyBefore([Delegate](ArgTypes&&... Arguments) { Delegate.Execute(Forward<ArgTypes>(Arguments)...); });
	}));
}
