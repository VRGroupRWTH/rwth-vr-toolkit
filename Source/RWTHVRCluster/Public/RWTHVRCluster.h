#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ClusterConsole/ClusterConsole.h"

class FRWTHVRClusterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FClusterConsole ClusterConsole;
};
