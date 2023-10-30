// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "InputExtensionInterface.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

/**
 * Simple interface that can be implemented if your class/component defines input actions.
 * Use SetupPlayerInput to bind actions, it will be called by VirtualRealityPawn::SetupPlayerInputComponent. 
 */
UINTERFACE(MinimalAPI, NotBlueprintable)
class UInputExtensionInterface : public UInterface
{
	GENERATED_BODY()
};

class RWTHVRTOOLKIT_API IInputExtensionInterface
{
	GENERATED_BODY()

public:

	// Called by VirtualRealityPawn::SetupPlayerInputComponent
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) {}

	// Helper function to get the local player subsystem 
	virtual UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputLocalPlayerSubsystem(const APawn* Pawn) const;
	
};
