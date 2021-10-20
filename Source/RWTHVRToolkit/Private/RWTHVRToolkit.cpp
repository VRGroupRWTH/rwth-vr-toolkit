#include "RWTHVRToolkit.h"

#include "ILiveLinkClient.h"
#include "LiveLinkClient.h"
#include "RWTHVRToolkitSettings.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
	#include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule ()
{
	ConsoleActivation.Register();

	FCoreDelegates::OnFEngineLoopInitComplete.AddRaw(this, &FRWTHVRToolkitModule::OnEngineLoopInitComplete);

#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
			"Project", "Plugins", "RWTH VR Toolkit",
			LOCTEXT("RWTHVRToolkitSettingsName", "RWTH VR Toolkit"),
			LOCTEXT("RWTHVRToolkitSettingsDescription", "Configure the RWTH VR Toolkit."),
			GetMutableDefault<URWTHVRToolkitSettings>()
		);
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FRWTHVRToolkitModule::HandleSettingsSaved);
		}
	}
#endif
}

void FRWTHVRToolkitModule::ShutdownModule()
{
	ConsoleActivation.Unregister();
	
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "RWTH VR Toolkit");
	}
#endif
}

void FRWTHVRToolkitModule::OnEngineLoopInitComplete()
{
	ApplyDefaultPreset();
}

bool FRWTHVRToolkitModule::HandleSettingsSaved()
{
	ApplyDefaultPreset();
	return true;
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