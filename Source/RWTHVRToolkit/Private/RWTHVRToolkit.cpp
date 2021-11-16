#include "RWTHVRToolkit.h"

#include "ILiveLinkClient.h"
#include "LiveLinkClient.h"
#include "RWTHVRToolkitSettings.h"

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule ()
{
	ConsoleActivation.Register();

	FCoreDelegates::OnFEngineLoopInitComplete.AddRaw(this, &FRWTHVRToolkitModule::OnEngineLoopInitComplete);

#if WITH_EDITOR
	GetMutableDefault<URWTHVRToolkitSettings>()->OnSettingChanged().AddRaw(this, &FRWTHVRToolkitModule::HandleSettingsSaved);
#endif
}

void FRWTHVRToolkitModule::ShutdownModule()
{
	ConsoleActivation.Unregister();	
}

void FRWTHVRToolkitModule::OnEngineLoopInitComplete()
{
	ApplyDefaultPreset();
}

void FRWTHVRToolkitModule::HandleSettingsSaved(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent)
{
	ApplyDefaultPreset();
}

void FRWTHVRToolkitModule::ApplyDefaultPreset()
{
	ULiveLinkPreset* StartupPreset = GetDefault<URWTHVRToolkitSettings>()->DefaultLiveLinkPreset.LoadSynchronous();
	if (StartupPreset != nullptr)
	{
		SetLiveLinkPreset(StartupPreset);
	}
}

void FRWTHVRToolkitModule::SetLiveLinkPreset(ULiveLinkPreset* Preset)
{
	// We should check for a currently applied preset/sources now, and if it includes the ART one.
	// If it does, and is active, do not apply it again. 

	const FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	
	if (LiveLinkPreset == nullptr && LiveLinkClient.GetSources().Num() == 0 )
	{
		LiveLinkPreset = Preset;
		LiveLinkPreset->ApplyToClient();
	}
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRToolkitModule, RWTHVRToolkit)