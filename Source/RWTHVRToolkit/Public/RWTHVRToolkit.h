#pragma once

#include "CoreMinimal.h"
#include "Fixes/ActivateConsoleInShipping.h"


class FRWTHVRToolkitModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;
	
private:
	FActivateConsoleInShipping ConsoleActivation;	
};
