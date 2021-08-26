#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "ActivateConsoleInShipping.generated.h"

/**
 * This fixes activates the debugging console shipping builds. Unfortunately, it can't activate the auto-completion inside the console
 */
USTRUCT()
struct RWTHVRTOOLKIT_API FActivateConsoleInShipping
{
	GENERATED_BODY()

	void Register();
	void Unregister() const;
	
private:
	void OnSessionStart(UWorld*, const UWorld::InitializationValues) const;

	TDelegate<void (UWorld*, const UWorld::InitializationValues)> On_Post_World_Initialization_Delegate;

	FDelegateHandle StartHandle;
};
