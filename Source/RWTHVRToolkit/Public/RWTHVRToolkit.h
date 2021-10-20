#pragma once

#include "CoreMinimal.h"
#include "Fixes/ActivateConsoleInShipping.h"

class ULiveLinkPreset;

class FRWTHVRToolkitModule : public IModuleInterface
{
public:
	virtual void StartupModule () override;
	virtual void ShutdownModule() override;
	
	void SetLiveLinkPreset(ULiveLinkPreset* Preset);

private:

	void OnEngineLoopInitComplete();
	bool HandleSettingsSaved();
	void ApplyDefaultPreset();
	
	FActivateConsoleInShipping ConsoleActivation;	
	ULiveLinkPreset* LiveLinkPreset = nullptr;
};
