#include "Utility/RWTHVRUtilities.h"

#include "AudioDevice.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "Utility/RWTHVRClusterUtilities.h"
#endif


DEFINE_LOG_CATEGORY(Toolkit);

bool URWTHVRUtilities::IsDesktopMode() { return !IsRoomMountedMode() && !IsHeadMountedMode(); }

bool URWTHVRUtilities::IsHeadMountedMode()
{
	// In editor builds: checks for EdEngine->IsVRPreviewActive()
	// In packaged builds: checks for `-vr` in commandline or bStartInVR in UGeneralProjectSettings
	return FAudioDevice::CanUseVRAudioDevice();
}

bool URWTHVRUtilities::IsRoomMountedMode()
{
#if PLATFORM_SUPPORTS_CLUSTER
	return URWTHVRClusterUtilities::IsRoomMountedMode();
#else
	return false;
#endif
}

bool URWTHVRUtilities::IsPrimaryNode()
{
#if PLATFORM_SUPPORTS_CLUSTER
	return URWTHVRClusterUtilities::IsPrimaryNode();
#else
	return false;
#endif	
}

float URWTHVRUtilities::GetEyeDistance()
{
	if (IsHeadMountedMode())
	{
		return GEngine->XRSystem->GetHMDDevice()->GetInterpupillaryDistance();
	}
	return 0;
}

void URWTHVRUtilities::ShowErrorAndQuit(UWorld* WorldContext, const FString& Message)
{
	UE_LOG(Toolkit, Error, TEXT("%s"), *Message)
#if WITH_EDITOR
	const FText Title = FText::FromString(FString("RUNTIME ERROR"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), Title);
#endif
	UKismetSystemLibrary::QuitGame(WorldContext, nullptr, EQuitPreference::Quit, false);
}
