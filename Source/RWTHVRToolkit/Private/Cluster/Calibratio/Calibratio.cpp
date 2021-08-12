#include "Cluster/Calibratio/Calibratio.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "IDisplayCluster.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "Utility/VirtualRealityUtilities.h"

void FCalibratio::Register()
{
	/* Registering console command */
	CalibratioConsoleCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("Calibratio"), TEXT("Spawn an instance of calibratio"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray< FString >&)
	{
		if(UVirtualRealityUtilities::IsRoomMountedMode()){
			/* Emitting cluster event to spawn on all nodes in sync*/
			FDisplayClusterClusterEventJson ClusterEvent;
			ClusterEvent.Name = "CalibratioSpawn";
			ClusterEvent.Category = "CalibratioSpawner";
			IDisplayCluster::Get().GetClusterMgr()->EmitClusterEventJson(ClusterEvent, false);
		} else {
			SpawnCalibratio();
		}
	}));

	
	/* Register cluster event listening */
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventJsonListener::CreateLambda([](const FDisplayClusterClusterEventJson& Event)
		{
			if (Event.Category.Equals("CalibratioSpawner") && Event.Name.Equals("CalibratioSpawn"))
			{
				SpawnCalibratio();
			}
		});
		ClusterManager->AddClusterEventJsonListener(ClusterEventListenerDelegate);
	}
}

void FCalibratio::Unregister() const
{
	IConsoleManager::Get().UnregisterConsoleObject(CalibratioConsoleCommand);
	IDisplayCluster::Get().GetClusterMgr()->RemoveClusterEventJsonListener(ClusterEventListenerDelegate);
}

void FCalibratio::SpawnCalibratio()
{ 
	if (UGameplayStatics::GetActorOfClass(GEngine->GetCurrentPlayWorld(), ACalibratioActor::StaticClass()) != nullptr) return;
	GEngine->GetCurrentPlayWorld()->SpawnActor<ACalibratioActor>();
}
