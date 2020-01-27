#pragma once

#include "Cluster/DisplayClusterClusterEvent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Delegates/IntegerSequence.h"
#include "DisplayClusterEventParameterHelper.h"
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

	// Calls a previously registered delegate with automatic type conversion.
	// Only compatible with delegates registered with RegisterSyncedAutoTypedMethod().
	template <typename... ArgTypes>
	inline void CallAutoTypedMethodSynced(FString UniqueIdentifier, ArgTypes&&... Arguments)
	{
		// Convert the arguments into a character map and call the "normal" function
		CallMethodSynced(UniqueIdentifier, CreateParameterMap(Forward<ArgTypes>(Arguments)...));
	}

	// Registers a delegate with automatic type conversion.
	// Delegates registered with this function should only be called via CallAutoTypedMethodSynced().
	template <typename RetType, typename... ArgTypes>
	inline void RegisterSyncedAutoTypedMethod(FString UniqueIdentifier, const TBaseDelegate<RetType, ArgTypes...>& Delegate)
	{
		// Register to the event using a lambda that will do the translation from the string map to the real values.
		RegisterSyncedMethod(UniqueIdentifier, FSyncMethod::CreateLambda([Delegate](TMap<FString, FString> Parameters) {
			CallDelegateWithParameterMap(Delegate, Paremeters);
		}));
	}

private:
	FOnClusterEventListener ClusterEventListenerDelegate;
	virtual void HandleClusterEvent(const FDisplayClusterClusterEvent& Event);

	TMap<FString, FSyncMethod> SyncedFunctions;
};
