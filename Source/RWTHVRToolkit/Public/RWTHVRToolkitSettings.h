// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Engine/EngineTypes.h"
#include "Templates/SubclassOf.h"
#include "LiveLinkPreset.h"

#include "RWTHVRToolkitSettings.generated.h"


/**
 * Settings for LiveLink.
 */
UCLASS(config=Game, defaultconfig)
class RWTHVRTOOLKIT_API URWTHVRToolkitSettings : public UObject
{
	GENERATED_BODY()

public:
	URWTHVRToolkitSettings() = default;

public:

	/** The default preset that should be applied */
	UPROPERTY(config, EditAnywhere, Category = "LiveLink")
	TSoftObjectPtr<ULiveLinkPreset> DefaultLiveLinkPreset;	
};
