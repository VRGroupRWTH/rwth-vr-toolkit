#include "Utility/VirtualRealityUtilities.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Components/DisplayClusterCameraComponent.h"
#include "Config/IDisplayClusterConfigManager.h"
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

bool UVirtualRealityUtilities::IsCave()
{
	if(!IsRoomMountedMode()) return false;
	
	const UDisplayClusterConfigurationData* ClusterConfig = IDisplayCluster::Get().GetConfigMgr()->GetConfig();
	return ClusterConfig->CustomParameters.Contains("Hardware_Platform")
		&& ClusterConfig->CustomParameters.Find("Hardware_Platform")->Equals("aixcave", ESearchCase::IgnoreCase);
}

bool UVirtualRealityUtilities::IsTdw()
{
	if(!IsRoomMountedMode()) return false;
	
	const UDisplayClusterConfigurationData* ClusterConfig = IDisplayCluster::Get().GetConfigMgr()->GetConfig();
	return ClusterConfig->CustomParameters.Contains("Hardware_Platform")
		&& ClusterConfig->CustomParameters.Find("Hardware_Platform")->Equals("TiledDisplayWall", ESearchCase::IgnoreCase);
}

bool UVirtualRealityUtilities::IsRolv()
{
	if(!IsRoomMountedMode()) return false;
	
	const UDisplayClusterConfigurationData* ClusterConfig = IDisplayCluster::Get().GetConfigMgr()->GetConfig();
	return ClusterConfig->CustomParameters.Contains("Hardware_Platform")
		&& ClusterConfig->CustomParameters.Find("Hardware_Platform")->Equals("ROLV", ESearchCase::IgnoreCase);
}

/* Return true on the Master in cluster mode and in a normal desktop session. Otherwise false */
bool UVirtualRealityUtilities::IsMaster()
{
	if (!IDisplayCluster::IsAvailable()) 
	{
		return true;
	}
	IDisplayClusterClusterManager* Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager == nullptr)
	{
		return true; // if we are not in cluster mode, we are always the master
	}
	return Manager->IsMaster() || !Manager->IsSlave();
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
	ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
	return (RootActor) ? RootActor->GetDefaultCamera()->GetInterpupillaryDistance() : 0;
}

EDisplayClusterEyeStereoOffset UVirtualRealityUtilities::GetNodeEyeType()
{
	ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
	return (RootActor) ? RootActor->GetDefaultCamera()->GetStereoOffset() : EDisplayClusterEyeStereoOffset::None;
}

UDisplayClusterSceneComponent* UVirtualRealityUtilities::GetClusterComponent(const FString& Name)
{
	ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
	return (RootActor) ? RootActor->GetComponentById(Name) : nullptr;
}

UDisplayClusterSceneComponent* UVirtualRealityUtilities::GetNamedClusterComponent(const ENamedClusterComponent& Component)
{
	switch(Component)
	{
	case ENamedClusterComponent::NCC_CAVE_ORIGIN: return GetClusterComponent("cave_origin");
	case ENamedClusterComponent::NCC_CAVE_CENTER: return GetClusterComponent("cave_center");
	case ENamedClusterComponent::NCC_CAVE_LHT: return GetClusterComponent("left_hand_target");
	case ENamedClusterComponent::NCC_CAVE_RHT: return GetClusterComponent("right_hand_target");
	case ENamedClusterComponent::NCC_SHUTTERGLASSES: return GetClusterComponent("shutter_glasses");
	case ENamedClusterComponent::NCC_ROLV_ORIGIN: return GetClusterComponent("rolv_origin");
	case ENamedClusterComponent::NCC_FLYSTICK: return GetClusterComponent("flystick");
	case ENamedClusterComponent::NCC_TDW_ORIGIN: return GetClusterComponent("tdw_origin_floor");
	case ENamedClusterComponent::NCC_TDW_CENTER: return GetClusterComponent("tdw_center");
	case ENamedClusterComponent::NCC_CALIBRATIO: return GetClusterComponent("calibratio");
	case ENamedClusterComponent::NCC_TRACKING_ORIGIN:
		UDisplayClusterSceneComponent* Result;
		if((Result = GetClusterComponent("cave_origin"))) return Result;
		if((Result = GetClusterComponent("rolv_origin"))) return Result;
		if((Result = GetClusterComponent("tdw_origin_floor"))) return Result;
		return nullptr;
	default: return nullptr;
	}
}

template <typename T>
bool UVirtualRealityUtilities::LoadAsset(const FString& Path, T* & Result)
{
	ConstructorHelpers::FObjectFinder<T> Loader(*Path);
	Result = Loader.Object;
	if (!Loader.Succeeded()) UE_LOG(LogTemp, Error, TEXT("Could not find %s. Have you renamed it?"), *Path);
	return Loader.Succeeded();
}

template <typename T>
bool UVirtualRealityUtilities::LoadClass(const FString& Path, TSubclassOf<T> & Result)
{
    ConstructorHelpers::FClassFinder<T> Loader(*Path);
	Result = Loader.Class;
	if (!Loader.Succeeded()) UE_LOG(LogTemp, Error, TEXT("Could not find %s. Have you renamed it?"), *Path);
	return Loader.Succeeded();
}
