#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "CalibratioActor.h"
#include "Cluster/IDisplayClusterClusterManager.h"

class FCalibratioModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

public:
	static void SpawnCalibratio();

private:
	IConsoleCommand* CalibratioConsoleCommand = nullptr;
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
};
