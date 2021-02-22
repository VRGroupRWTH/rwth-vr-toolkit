#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "FixNDisplayStereoDevice.generated.h"

/**
 * This fixes the behavior of nDisplay DisplayClusterRenderManager, which resets the global stereo render device
 * This is problematic, since it resets the pointer (L128) to the StereoRenderDevice, even if it does not re-initialize (L113) it.
 * This results in a fine first start, but crashes on the second VR start. For Desktop this seems to still function fine.
 *
 * For this behavior a Pull-Request is in progress. See https://github.com/EpicGames/UnrealEngine/pull/7738
 */
USTRUCT()
struct DISPLAYCLUSTEREXTENSIONS_API FFixNDisplayStereoDevice
{
	GENERATED_BODY()

	void Register();
	void Unregister();
	
private:
	void OnSessionStart(UWorld*, const UWorld::InitializationValues);
	void OnSessionEnd(UWorld*);

	TSharedPtr< class IStereoRendering, ESPMode::ThreadSafe > StoredRenderingDevice;

	TDelegate<void (UWorld*, const UWorld::InitializationValues)> On_Post_World_Initialization_Delegate;
	TDelegate<void (UWorld*)> On_Pre_World_Finish_Destroy_Delegate;

	FDelegateHandle StartHandle;
	FDelegateHandle EndHandle;
};
