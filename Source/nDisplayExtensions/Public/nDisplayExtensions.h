#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class NDISPLAYEXTENSIONS_API FnDisplayExtensionsModule : public IModuleInterface
{
public:
  virtual void StartupModule () override;
  virtual void ShutdownModule() override;
};
