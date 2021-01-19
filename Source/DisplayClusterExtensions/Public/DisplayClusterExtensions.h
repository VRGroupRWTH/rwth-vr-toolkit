#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ClusterConsole.h"

class FDisplayClusterExtensionsModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

	FClusterConsole ClusterConsole;
};
