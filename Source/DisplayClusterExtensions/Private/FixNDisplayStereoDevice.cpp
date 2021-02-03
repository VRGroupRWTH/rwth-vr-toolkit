#include "FixNDisplayStereoDevice.h"

void FFixNDisplayStereoDevice::Register()
{
	On_Post_World_Initialization_Delegate.BindRaw(this, &FFixNDisplayStereoDevice::OnSessionStart);
	StartHandle = FWorldDelegates::OnPostWorldInitialization.Add(On_Post_World_Initialization_Delegate);

	On_Pre_World_Finish_Destroy_Delegate.BindRaw(this, &FFixNDisplayStereoDevice::OnSessionEnd);
	EndHandle = FWorldDelegates::OnPreWorldFinishDestroy.Add(On_Pre_World_Finish_Destroy_Delegate);
}

void FFixNDisplayStereoDevice::Unregister()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(StartHandle);
	FWorldDelegates::OnPreWorldFinishDestroy.Remove(EndHandle);
}

void FFixNDisplayStereoDevice::OnSessionStart(UWorld*, const UWorld::InitializationValues)
{
	/* Store handle before it is released */
	if(GEngine)
	{
		StoredRenderingDevice = GEngine->StereoRenderingDevice;
	}
}

void FFixNDisplayStereoDevice::OnSessionEnd(UWorld*)
{
	/* Restore handle after it was released */
	if(GEngine)
	{
		GEngine->StereoRenderingDevice = StoredRenderingDevice;
	}
}
