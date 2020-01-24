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

template <typename... Values>
struct FillParameterMapImpl;

template <typename CurrentValueType, typename... RemainingValueTypes>
struct FillParameterMapImpl<CurrentValueType, RemainingValueTypes...>
{
	template <int ArgumentIndex>
	static inline void Invoke(
		TMap<FString, FString>* ParameterMap, const CurrentValueType& CurrentValue, RemainingValueTypes&&... RemainingValues)
	{
		TArray<uint8> bytes;
		FMemoryWriter writer(bytes);
		// Const cast is necessary here because the "<<" operator is used for reading and writing!
		auto NonConstCurrentValue = const_cast<CurrentValueType&>(CurrentValue);
		writer << NonConstCurrentValue;
		FString value;
		for (const uint8 byte : bytes)
		{
			value += static_cast<TCHAR>(byte + 1);	  // TODO: Preallocate!
		}
		ParameterMap->Add(FString::FromInt(ArgumentIndex), value);
		FillParameterMapImpl<RemainingValues...>::Invoke<ArgumentIndex + 1>(
			ParameterMap, RemainingValues...);	  // TODO: forward remaining values!
	}
};

template <>
struct FillParameterMapImpl<>
{
	template <int ArgumentIndex>
	static inline void Invoke(TMap<FString, FString>* ParameterMap)
	{
	}
};

template <typename... ArgTypes>
void FillParameterMap(TMap<FString, FString>* ParameterMap, ArgTypes&&... Arguments)
{
	FillParameterMapImpl<ArgTypes...>::Invoke<0>(ParameterMap, Arguments...);	 // TODO: forward arguments!
}

template <typename... ArgTypes>
void UClusterSyncedMethodCaller::CallAutoTypedDelegate(FString UniqueIdentifier, ArgTypes&&... Arguments)
{
	TMap<FString, FString> ParameterMap;
	// TODO: Add inline constrcution with a initializer list for performance reasons?
	//       However this is possibly be super complicated
	FillParameterMap(&ParameterMap, Forward<ArgTypes>(Arguments)...);
	CallMethodSynced(UniqueIdentifier, ParameterMap);
}

template <typename RetType, typename DelegateType, typename TupleType, int... Indices>
void InvokeDelegateUsingArgumentTupleImpl(
	const DelegateType& Delegate, const TupleType& ArgumentTuple, TIntegerSequence<int, Indices...>)
{
	Delegate.Execute(ArgumentTuple.Get<Indices>()...);	  // TODO: Add Forward<>() here!
}

// This can be replaces with tuple.ApplyBefore/After
template <typename TupleType, typename RetType, typename... ArgTypes>
RetType InvokeDelegateUsingArgumentTuple(const TBaseDelegate<RetType, ArgTypes...>& Delegate, const TupleType& ArgumentTuple)
{
	return InvokeDelegateUsingArgumentTupleImpl<RetType>(Forward<const TBaseDelegate<RetType, ArgTypes...>&>(Delegate),
		Forward<const TupleType&>(ArgumentTuple), TMakeIntegerSequence<int, sizeof...(ArgTypes)>{});
}

template <int CurrentIndex, typename... ArgTypes>
inline void FillArgumentTuple(TTuple<ArgTypes...>* ArgumentTuple, const TMap<FString, FString>& Parameters)
{
	constexpr auto ArgumentCount = sizeof...(ArgTypes);
	if (CurrentIndex < ArgumentCount)
	{
		const auto& DataString = Parameters[FString::FromInt(CurrentIndex)];
		TArray<uint8> Bytes;	// TODO: Preallocate

		for (const auto Character : DataString)
		{
			Bytes.Add(static_cast<uint8>(Character - 1));
		}

		FMemoryReader Reader(Bytes);
		Reader << ArgumentTuple->Get<CurrentIndex>();
		// FillArgumentTuple<CurrentIndex + 1>(ArgumentTuple, Parameters); // TODO: Fix this!
	}
}

template <typename RetType, typename... ArgTypes>
void UClusterSyncedMethodCaller::RegisterAutoTypedDelegate(
	FString UniqueIdentifier, const TBaseDelegate<RetType, ArgTypes...>& Delegate)
{
	RegisterSyncedMethod(UniqueIdentifier, FSyncMethod::CreateLambda([Delegate](TMap<FString, FString> Parameters) {
		TTuple<typename TRemoveCV<typename TRemoveReference<ArgTypes>::Type>::Type...> ArgumentTuple;
		FillArgumentTuple<0>(&ArgumentTuple, Parameters);
		InvokeDelegateUsingArgumentTuple(Delegate, ArgumentTuple);
	}));
}
