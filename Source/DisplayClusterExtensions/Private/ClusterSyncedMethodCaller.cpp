#include "ClusterSyncedMethodCaller.h"

#include "IDisplayCluster.h"

void UClusterSyncedMethodCaller::RegisterListener()
{
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventListener::CreateUObject(this, &UClusterSyncedMethodCaller::HandleClusterEvent);
		ClusterManager->AddClusterEventListener(ClusterEventListenerDelegate);
	}

}

void UClusterSyncedMethodCaller::DeregisterListener() {
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && ClusterEventListenerDelegate.IsBound())
	{
		ClusterManager->RemoveClusterEventListener(ClusterEventListenerDelegate);
	}
}

void UClusterSyncedMethodCaller::CallMethodSynced(FSyncMethod MethodToCall, TMap<FString, FString> Parameters, FString UniqueIdentifier)
{
	IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager)
	{
		if (Manager->IsStandalone()) {
			//in standalone (e.g., desktop editor play) cluster events are not executed....
			MethodToCall.Execute(Parameters);
		}
		else {
			// else create a cluster event to react to
			FDisplayClusterClusterEvent cluster_event;
			cluster_event.Category = "ClusterSyncedMethodCaller";
			cluster_event.Type = "ClusterSyncedMethodCaller";
			cluster_event.Name = UniqueIdentifier;
			cluster_event.Parameters = Parameters;
			Manager->EmitClusterEvent(cluster_event, true);

			SyncedFunctions.Add(UniqueIdentifier, MethodToCall);
		}
	}
}

void UClusterSyncedMethodCaller::HandleClusterEvent(const FDisplayClusterClusterEvent & Event)
{
	if (Event.Category.Equals("ClusterSyncedMethodCaller") && Event.Type.Equals("ClusterSyncedMethodCaller")) {
		if (SyncedFunctions.Contains(Event.Name)) {
			SyncedFunctions.FindChecked(Event.Name).Execute(Event.Parameters);
		}
	}
}
