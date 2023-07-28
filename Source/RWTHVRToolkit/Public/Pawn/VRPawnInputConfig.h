// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "VRPawnInputConfig.generated.h"

/**
 * 
 */
UCLASS()
class RWTHVRTOOLKIT_API UVRPawnInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* MoveUp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* DesktopRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* Fire;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* Grab;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* ToggleNavigationMode;
	


};
