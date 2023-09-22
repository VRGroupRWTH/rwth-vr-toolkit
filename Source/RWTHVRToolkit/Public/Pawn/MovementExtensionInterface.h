// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "MovementExtensionInterface.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
/**
 * 
 */
UINTERFACE(MinimalAPI, NotBlueprintable)
class UMovementExtensionInterface : public UInterface
{
	GENERATED_BODY()
};

class RWTHVRTOOLKIT_API IMovementExtensionInterface
{
	GENERATED_BODY()

public:

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) {}

	virtual UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputLocalPlayerSubsystem(APawn* Pawn) const;
	
};
