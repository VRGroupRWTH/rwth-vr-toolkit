#include "VirtualRealityUtilities.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Components/DisplayClusterCameraComponent.h"
#include "DisplayClusterRootActor.h"
#include "DisplayClusterConfigurationTypes.h"
#include "Engine/Engine.h"
#include "Game/IDisplayClusterGameManager.h"
#include "IDisplayCluster.h"
#include "IXRTrackingSystem.h"

bool UVirtualRealityUtilities::IsDesktopMode()
{
	return !IsRoomMountedMode() && !IsHeadMountedMode();
}
bool UVirtualRealityUtilities::IsRoomMountedMode()
{
	return IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster;
}
bool UVirtualRealityUtilities::IsHeadMountedMode()
{
	return GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
}

bool UVirtualRealityUtilities::IsMaster()
{
	IDisplayClusterClusterManager* manager = IDisplayCluster::Get().GetClusterMgr();
	if (manager == nullptr) // no manager means we are not in clustermode and therefore master
		return true;

	return manager->IsMaster();
}

bool UVirtualRealityUtilities::IsSlave()
{
	return !IsMaster();
}

FString UVirtualRealityUtilities::GetNodeName()
{
	return IsRoomMountedMode() ? IDisplayCluster::Get().GetClusterMgr()->GetNodeId() : FString(TEXT("Localhost"));
}
float UVirtualRealityUtilities::GetEyeDistance()
{
	return IDisplayCluster::Get().GetGameMgr()->GetRootActor()->GetDefaultCamera()->GetInterpupillaryDistance();
}

EDisplayClusterEyeStereoOffset UVirtualRealityUtilities::GetNodeEyeType()
{
	return IDisplayCluster::Get().GetGameMgr()->GetRootActor()->GetDefaultCamera()->GetStereoOffset();
}

UDisplayClusterSceneComponent* UVirtualRealityUtilities::GetClusterComponent(const FString& Name)
{
	return IDisplayCluster::Get().GetGameMgr()->GetRootActor()->GetComponentById(Name);
}
