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

	//This will call MethodToCall in a synced manner with parameters, also works in non-cluster mode, notive that in this case this call is synced between all nodes, but async to the call of this method!
	//UniqueIdentifier should be unique to any call of this method of any object!
	//template<class UserClass>
	//void CallMethodSynced(UserClass* UserObject, void(UserClass::*Function)(TMap<FString, FString>), TMap<FString, FString> parameters, FString unique_identifier);
	void CallMethodSynced(FSyncMethod MethodToCall, TMap<FString, FString> Parameters, FString UniqueIdentifier);
  
private:
	FOnClusterEventListener ClusterEventListenerDelegate;
	virtual void HandleClusterEvent(const FDisplayClusterClusterEvent& Event);

	TMap<FString, FSyncMethod> SyncedFunctions;

};

