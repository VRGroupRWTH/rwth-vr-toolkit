#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "CAVEOverlay/CAVEOverlaySettings.h"
#include "CAVEOverlay/CAVEOverlayController.h"
#include "Kismet/GameplayStatics.h"
#include "CAVEOverlay.generated.h"

/**
 * Adds the warning tape, which appears if the user gets too close to the wall for the aixCAVE
 */
USTRUCT()
struct RWTHVRCLUSTER_API FCAVEOverlay
{
	GENERATED_BODY()

	void Register();
	void Unregister() const;

private:
	TDelegate<void(UWorld*, const UWorld::InitializationValues)> On_Post_World_Initialization_Delegate;
	void OnSessionStart(UWorld* World, UWorld::InitializationValues);
	FDelegateHandle SessionStartDelegate;
};
