#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "ModuleManager.h"

class FDisplayClusterExtensionsEditorModule : public IModuleInterface
{
public:
	// Begin IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface implementation

};