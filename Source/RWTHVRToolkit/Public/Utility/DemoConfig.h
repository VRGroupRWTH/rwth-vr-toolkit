#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DemoConfig.generated.h"

/**
 * This DeveloperSettings derivative has the following behavior:
 * 1. Creates a DefaultDemo.ini in your projects config folder (Name can be changed via "config =" parameter in UCLASS)
 * 2. Creates a settings page in the project settings (ProjectSettings|Game|Demo) for all UPROPERTY-s and stores all UPROPERTY-s with a "Config" flag into (1.), that are altered
 * 3. When packaged, creates a Demo.ini (without "Default") in the config directory of the game and dumps the contents of the DefaultDemo.ini into it
 * 4. When packaged, loads the config file from (3.) into this class
 * 5. Settings can be accessed from everywhere via `GetDefault<UYourSubClass>()->PropertyName`
 */
UCLASS(Abstract, config = Demo, defaultconfig)
class RWTHVRTOOLKIT_API UDemoConfig : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual FName GetCategoryName() const override {return "Game";};

	#if WITH_EDITOR
	    virtual FText GetSectionText() const override {return FText::FromString("Demo");};
    #endif

	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();

		// Do not create/load this config file in the editor. We have the DefaultDemo.ini already
		if(FApp::GetBuildTargetType() == EBuildTargetType::Editor) return; 

		// Load config file (does nothing if not exist)
		const FString ConfigFile = FPaths::Combine(FPaths::ProjectConfigDir(), FPaths::GetCleanFilename(GetClass()->GetConfigName()));
	    FConfigCacheIni Config(EConfigCacheType::DiskBacked);
		Config.LoadFile(ConfigFile);

		// Check existence of correct section (fails if file does not exist)
		if(Config.DoesSectionExist(*GetClass()->GetPathName(), ConfigFile))
		{
		    LoadConfig(GetClass(), *ConfigFile);
        }
	    else
        {
		    SaveConfig(CPF_Config, *ConfigFile, &Config); 
			Config.Flush(false);
        }
	};
};
