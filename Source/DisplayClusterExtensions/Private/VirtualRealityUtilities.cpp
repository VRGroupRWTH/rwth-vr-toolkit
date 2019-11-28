#include "VirtualRealityUtilities.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/Engine.h"
#include "DisplayClusterClusterEvent.h"
#include "Game/IDisplayClusterGameManager.h"
#include "IDisplayCluster.h"
#include "IDisplayClusterConfigManager.h"
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

FString UVirtualRealityUtilities::GetNodeName()
{
	return IsRoomMountedMode() ? IDisplayCluster::Get().GetClusterMgr()->GetNodeId() : FString(TEXT("Localhost"));
}
float UVirtualRealityUtilities::GetEyeDistance()
{
	return IDisplayCluster::Get().GetConfigMgr()->GetConfigStereo().EyeDist;
}

EEyeType UVirtualRealityUtilities::GetNodeEyeType()
{
	FDisplayClusterConfigClusterNode CurrentNodeConfig;
	IDisplayCluster::Get().GetConfigMgr()->GetClusterNode(GetNodeName(), CurrentNodeConfig);

	FString s = CurrentNodeConfig.ToString();

	if (s.Contains("mono_eye"))
	{
		TArray<FString> stringArray;
		int32 count = s.ParseIntoArray(stringArray, TEXT(","));
		for (int x = 0; x < count; x++)
		{
			if (!stringArray[x].Contains("mono_eye")) continue;
			if (stringArray[x].Contains("left"))
			{
				return EEyeType::ET_STEREO_LEFT;
			}
			if (stringArray[x].Contains("right"))
			{
				return EEyeType::ET_STEREO_RIGHT;
			}
		}
	}
	else
	{
		return EEyeType::ET_MONO;
	}
	return EEyeType::ET_MONO;
}

UDisplayClusterSceneComponent* UVirtualRealityUtilities::GetClusterComponent(const FString& Name)
{
	return IDisplayCluster::Get().GetGameMgr()->GetNodeById(Name);
}

void UVirtualRealityUtilities::ClusterExecute(const FString& Command)
{
	FDisplayClusterClusterEvent event;
	event.Name = "NDisplayCMD: " + Command;
	event.Type = "NDisplayCMD";
	event.Category = "VRPawn";
	event.Parameters.Add("Command", Command);
	IDisplayCluster::Get().GetClusterMgr()->EmitClusterEvent(event, false);
}
