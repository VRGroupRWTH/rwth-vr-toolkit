#pragma once

#include "CoreMinimal.h"
#include "Fixes/FixNDisplayStereoDevice.h"
#include "Modules/ModuleManager.h"
#include "ClusterConsole.h"


class FDisplayClusterExtensionsModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;

private:
	FClusterConsole ClusterConsole;
	FFixNDisplayStereoDevice StereoDeviceFix;
};
