#include "Utility/VirtualRealityUtilities.h"

#if PLATFORM_SUPPORTS_NDISPLAY
#include "DisplayClusterConfigurationTypes.h"
#include "DisplayClusterRootActor.h"
#include "IDisplayCluster.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Components/DisplayClusterCameraComponent.h"
#include "Config/IDisplayClusterConfigManager.h"
#include "Game/IDisplayClusterGameManager.h"
#endif

#include "AudioDevice.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"


DEFINE_LOG_CATEGORY(Toolkit);

bool UVirtualRealityUtilities::IsDesktopMode()
{
	return !IsRoomMountedMode() && !IsHeadMountedMode();
}

bool UVirtualRealityUtilities::IsRoomMountedMode()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	return IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster;
#else
	return false;
#endif
}

bool UVirtualRealityUtilities::IsHeadMountedMode()
{
	// In editor builds: checks for EdEngine->IsVRPreviewActive()
	// In packaged builds: checks for `-vr` in commandline or bStartInVR in UGeneralProjectSettings
	return FAudioDevice::CanUseVRAudioDevice();
}

bool UVirtualRealityUtilities::IsCave()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	if (!IsRoomMountedMode())
		return false;

	const UDisplayClusterConfigurationData* ClusterConfig = IDisplayCluster::Get().GetConfigMgr()->GetConfig();
	return ClusterConfig->CustomParameters.Contains("Hardware_Platform")
		&& ClusterConfig->CustomParameters.Find("Hardware_Platform")->Equals("aixcave", ESearchCase::IgnoreCase);
#else
	return false;
#endif
}

bool UVirtualRealityUtilities::IsRolv()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	if (!IsRoomMountedMode())
		return false;

	const UDisplayClusterConfigurationData* ClusterConfig = IDisplayCluster::Get().GetConfigMgr()->GetConfig();
	return ClusterConfig->CustomParameters.Contains("Hardware_Platform")
		&& ClusterConfig->CustomParameters.Find("Hardware_Platform")->Equals("ROLV", ESearchCase::IgnoreCase);
#else
	return false;
#endif
}

/* Return true on the Primary in cluster mode and in a normal desktop session. Otherwise false */
bool UVirtualRealityUtilities::IsPrimaryNode()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	if (!IDisplayCluster::IsAvailable())
	{
		return true;
	}
	IDisplayClusterClusterManager* Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager == nullptr)
	{
		return true; // if we are not in cluster mode, we are always the primary node
	}
	return Manager->IsPrimary() || !Manager->IsSecondary();
#else
    return true;
#endif
}

bool UVirtualRealityUtilities::IsSecondaryNode()
{
	return !IsPrimaryNode();
}

FString UVirtualRealityUtilities::GetNodeName()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	return IsRoomMountedMode() ? IDisplayCluster::Get().GetClusterMgr()->GetNodeId() : FString(TEXT("Localhost"));
#else
	return FString(TEXT("Localhost"));
#endif
}

float UVirtualRealityUtilities::GetEyeDistance()
{
	if (IsHeadMountedMode())
	{
		return GEngine->XRSystem->GetHMDDevice()->GetInterpupillaryDistance();
	}
	else
	{
#if PLATFORM_SUPPORTS_NDISPLAY
		const ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
		return (RootActor) ? RootActor->GetDefaultCamera()->GetInterpupillaryDistance() : 0.0f;
#else
	    return 0.0f;
#endif
	}
}

EEyeStereoOffset UVirtualRealityUtilities::GetNodeEyeType()
{
#if PLATFORM_SUPPORTS_NDISPLAY
	const ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
	return static_cast<EEyeStereoOffset>((RootActor)
		                                     ? RootActor->GetDefaultCamera()->GetStereoOffset()
		                                     : EDisplayClusterEyeStereoOffset::None);
#else
	return EDisplayClusterEyeStereoOffset::None;
#endif
}

USceneComponent* UVirtualRealityUtilities::GetClusterComponent(const FString& Name)
{
#if PLATFORM_SUPPORTS_NDISPLAY
	const ADisplayClusterRootActor* RootActor = IDisplayCluster::Get().GetGameMgr()->GetRootActor();
	return (RootActor) ? RootActor->GetComponentByName<USceneComponent>(Name) : nullptr;
#else
	return nullptr;
#endif
}

USceneComponent* UVirtualRealityUtilities::GetNamedClusterComponent(const ENamedClusterComponent& Component)
{
	switch (Component)
	{
	case ENamedClusterComponent::NCC_CAVE_ORIGIN:
		return GetClusterComponent("cave_origin");
	case ENamedClusterComponent::NCC_CAVE_CENTER:
		return GetClusterComponent("cave_center");
	case ENamedClusterComponent::NCC_CAVE_LHT:
		return GetClusterComponent("left_hand_target");
	case ENamedClusterComponent::NCC_CAVE_RHT:
		return GetClusterComponent("right_hand_target");
	case ENamedClusterComponent::NCC_SHUTTERGLASSES:
		return GetClusterComponent("shutter_glasses");
	case ENamedClusterComponent::NCC_ROLV_ORIGIN:
		return GetClusterComponent("rolv_origin");
	case ENamedClusterComponent::NCC_FLYSTICK:
		return GetClusterComponent("flystick");
	case ENamedClusterComponent::NCC_CALIBRATIO:
		return GetClusterComponent("calibratio");
	case ENamedClusterComponent::NCC_TRACKING_ORIGIN:
		USceneComponent* Result;
		if ((Result = GetClusterComponent("cave_origin")))
			return Result;
		if ((Result = GetClusterComponent("rolv_origin")))
			return Result;
		if ((Result = GetClusterComponent("tdw_origin_floor")))
			return Result;
		return nullptr;
	default:
		return nullptr;
	}
}

void UVirtualRealityUtilities::ShowErrorAndQuit(UWorld* WorldContext, const FString& Message)
{
	UE_LOG(Toolkit, Error, TEXT("%s"), *Message)
#if WITH_EDITOR
	const FText Title = FText::FromString(FString("RUNTIME ERROR"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), Title);
#endif
	UKismetSystemLibrary::QuitGame(WorldContext, nullptr, EQuitPreference::Quit, false);
}
