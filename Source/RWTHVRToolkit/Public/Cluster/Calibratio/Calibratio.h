#pragma once

#include "CoreMinimal.h"
#include "CalibratioActor.h"
#include "Cluster/IDisplayClusterClusterManager.h"

#include "Calibratio.generated.h"

/* This class is used to spawn and despawn the calibratio actor, if a console command arrives */
USTRUCT()
struct RWTHVRTOOLKIT_API FCalibratio
{
	GENERATED_BODY()
	
	void Register();
	void Unregister() const;

public:
	static void SpawnCalibratio();

private:
	IConsoleCommand* CalibratioConsoleCommand = nullptr;
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
};
