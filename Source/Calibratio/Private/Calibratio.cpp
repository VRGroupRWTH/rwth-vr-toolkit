#include "Calibratio.h"
#include "Kismet/GameplayStatics.h"
#include "IDisplayCluster.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "Utility/VirtualRealityUtilities.h"

#define LOCTEXT_NAMESPACE "FCalibratioModule"

void FCalibratioModule::StartupModule ()
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
void FCalibratioModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(CalibratioConsoleCommand);
	IDisplayCluster::Get().GetClusterMgr()->RemoveClusterEventJsonListener(ClusterEventListenerDelegate);
}

void FCalibratioModule::SpawnCalibratio()
{ 
	if (UGameplayStatics::GetActorOfClass(GEngine->GetCurrentPlayWorld(), ACalibratioActor::StaticClass()) != nullptr) return;
	GEngine->GetCurrentPlayWorld()->SpawnActor<ACalibratioActor>();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCalibratioModule, Calibratio)