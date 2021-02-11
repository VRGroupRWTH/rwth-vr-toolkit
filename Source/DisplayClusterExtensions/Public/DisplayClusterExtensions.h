#pragma once

#include "CoreMinimal.h"
#include "Cluster/CAVEOverlay/CAVEOverlay.h"
#include "Fixes/FixNDisplayStereoDevice.h"
#include "Modules/ModuleManager.h"


class FDisplayClusterExtensionsModule : public IModuleInterface
{
public:
  virtual void StartupModule () override;
  virtual void ShutdownModule() override;

private:
	FFixNDisplayStereoDevice StereoDeviceFix;
	FCAVEOverlay CAVEOverlay;
};
