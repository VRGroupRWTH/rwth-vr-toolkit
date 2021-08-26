#pragma once

#include "CoreMinimal.h"
#include "CAVEOverlay/CAVEOverlay.h"
#include "Modules/ModuleManager.h"
#include "ClusterConsole.h"

class FRWTHVRClusterModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

private:
	FClusterConsole ClusterConsole;
	FCAVEOverlay CAVEOverlay;
};
