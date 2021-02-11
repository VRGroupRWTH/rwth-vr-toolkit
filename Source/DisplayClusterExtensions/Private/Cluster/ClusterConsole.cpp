#include "ClusterConsole.h"
#include "IDisplayCluster.h"
#include "Cluster/DisplayClusterClusterEvent.h"

void FClusterConsole::Register()
{
	/* Registering console command */
	ClusterConsoleCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ClusterExecute"), TEXT("<Your Command> - Execute commands on every node of the nDisplay cluster by prepending ClusterExecute"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray< FString >& Args)
	{
		if(IDisplayCluster::Get().GetClusterMgr() == nullptr) return;
		
		/* Emitting cluster event */
		FDisplayClusterClusterEventJson ClusterEvent;
		ClusterEvent.Name = "ClusterExecute " + Args[0];
		ClusterEvent.Type = Args[0];
		ClusterEvent.Category = "NDisplayClusterExecute";
		ClusterEvent.Parameters.Add("Command", FString::Join(Args, TEXT(" ")));
		
		IDisplayCluster::Get().GetClusterMgr()->EmitClusterEventJson(ClusterEvent, false);
	}));

	/* Register cluster event handling */
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventJsonListener::CreateLambda([](const FDisplayClusterClusterEventJson& Event)
		{		
			/* Actual handling */
			if (Event.Category.Equals("NDisplayClusterExecute") && Event.Parameters.Contains("Command") && GEngine)
			{
				GEngine->Exec(GEngine->GetCurrentPlayWorld(), *Event.Parameters["Command"]);
			}
		});
		ClusterManager->AddClusterEventJsonListener(ClusterEventListenerDelegate);
	}
}

void FClusterConsole::Unregister()
{
	IConsoleManager::Get().UnregisterConsoleObject(ClusterConsoleCommand);
	IDisplayCluster::Get().GetClusterMgr()->RemoveClusterEventJsonListener(ClusterEventListenerDelegate);
}

