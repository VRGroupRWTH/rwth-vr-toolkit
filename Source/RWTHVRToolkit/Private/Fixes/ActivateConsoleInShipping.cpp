#include "Fixes/ActivateConsoleInShipping.h"
#include "Engine/Console.h"

void FActivateConsoleInShipping::Register()
{
	if(FApp::GetBuildConfiguration() != EBuildConfiguration::Shipping) return; /* Should only enable console in shipping */

	On_Post_World_Initialization_Delegate.BindRaw(this, &FActivateConsoleInShipping::OnSessionStart);
	StartHandle = FWorldDelegates::OnPostWorldInitialization.Add(On_Post_World_Initialization_Delegate);
}

void FActivateConsoleInShipping::Unregister() const
{
	if(FApp::GetBuildConfiguration() != EBuildConfiguration::Shipping) return;

    FWorldDelegates::OnPostWorldInitialization.Remove(StartHandle);
}

void FActivateConsoleInShipping::OnSessionStart(UWorld* World, const UWorld::InitializationValues Values) const
{
    if(!World->IsGameWorld() || !World->GetGameViewport() || World->GetGameViewport()->ViewportConsole != nullptr) return;

    World->GetGameViewport()->ViewportConsole = NewObject<UConsole>(World->GetGameViewport());
}
