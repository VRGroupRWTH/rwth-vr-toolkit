#pragma once

#include "CoreMinimal.h"
#include "Fixes/ActivateConsoleInShipping.h"
#include "Fixes/LiveLinkMotionControllerFix.h"

// Master switch for the CAVE nDisplay secondary-node sync investigation logs
// ([SYNC-SET-CHECK], [DIRTY], [CLUSTER-DCRA]). Set to 1 to re-enable all of them at once.
// Kept off in normal use since [SYNC-SET-CHECK]/[DIRTY] log per-frame during movement.
#define CAVE_SYNC_DEBUG_LOGS 0


class FRWTHVRToolkitModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FActivateConsoleInShipping ConsoleActivation;
	TUniquePtr<FLiveLinkMotionControllerFix> LiveLinkMotionController;
};
