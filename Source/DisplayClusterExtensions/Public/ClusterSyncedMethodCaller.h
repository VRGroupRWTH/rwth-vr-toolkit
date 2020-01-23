#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "Delegates/Delegate.h"

#include "ClusterSyncedMethodCaller.generated.h"

typedef TMap<FString, FString> FStringMap;
DECLARE_DELEGATE_OneParam(FSyncMethod, FStringMap)
//typedef FSyncMethod::FDelegate FSyncMethodDelegate;

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API UClusterSyncedMethodCaller : public UObject
{
	GENERATED_BODY()

public:
	//RegisterListener should be called in BeginPlay()
	void RegisterListener();

	//DeregisterListener() should be called in EndPlay()
	void DeregisterListener();

	//This will call a with UniqueIdentifier registered method in a synced manner with given parameters, also works in non-cluster mode.
	//Notice that in the cluster case this call is synced between all nodes, but async to the call of this method!
	void CallMethodSynced(FString UniqueIdentifier, TMap<FString, FString> Parameters);

	//Register a method to be called in a synced manner, UniqueIdentifier should be unique to any call of this method of any object!
	void RegisterSyncedMethod(FString UniqueIdentifier, FSyncMethod MethodToCall);
  
private:
	FOnClusterEventListener ClusterEventListenerDelegate;
	virtual void HandleClusterEvent(const FDisplayClusterClusterEvent& Event);

	TMap<FString, FSyncMethod> SyncedFunctions;

};

