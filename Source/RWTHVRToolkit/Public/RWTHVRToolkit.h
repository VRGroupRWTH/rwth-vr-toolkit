#pragma once

#include "CoreMinimal.h"
#include "Cluster/CAVEOverlay/CAVEOverlay.h"
#include "Modules/ModuleManager.h"
#include "Cluster/ClusterConsole.h"
#include "Fixes/ActivateConsoleInShipping.h"


class FRWTHVRToolkitModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

private:
	FClusterConsole ClusterConsole;
	FCAVEOverlay CAVEOverlay;
	FActivateConsoleInShipping ConsoleActivation;
};
