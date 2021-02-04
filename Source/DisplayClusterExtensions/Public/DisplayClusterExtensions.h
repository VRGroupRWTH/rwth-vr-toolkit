#pragma once

#include "CoreMinimal.h"
#include "CAVEOverlay/CAVEOverlay.h"
#include "FixNDisplayStereoDevice.h"
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
