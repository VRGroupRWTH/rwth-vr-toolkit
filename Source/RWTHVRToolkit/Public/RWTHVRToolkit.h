#pragma once

#include "CoreMinimal.h"
#include "Fixes/ActivateConsoleInShipping.h"
#include "Fixes/LiveLinkMotionControllerFix.h"


class FRWTHVRToolkitModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;
	
private:
	FActivateConsoleInShipping ConsoleActivation;
	TUniquePtr<FLiveLinkMotionControllerFix> LiveLinkMotionController;
};
