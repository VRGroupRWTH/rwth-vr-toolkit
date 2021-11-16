// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Engine/EngineTypes.h"
#include "Templates/SubclassOf.h"
#include "LiveLinkPreset.h"
#include "Utility/DemoConfig.h"

#include "RWTHVRToolkitSettings.generated.h"


/**
 * Settings for LiveLink.
 */
UCLASS(config=RWTHVRToolkit, defaultconfig)
class RWTHVRTOOLKIT_API URWTHVRToolkitSettings : public UDemoConfig
{
	GENERATED_BODY()

	virtual FName GetCategoryName() const override { return "Plugins"; };

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return FText::FromString("RWTH VR Toolkit"); };
#endif


public:
	URWTHVRToolkitSettings() = default;

public:

	/** The default preset that should be applied */
	UPROPERTY(config, EditAnywhere, Category = "LiveLink")
	TSoftObjectPtr<ULiveLinkPreset> DefaultLiveLinkPreset;	
};
