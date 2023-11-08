// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "MovementComponentBase.generated.h"

class AVirtualRealityPawn;
/**
 * Base component for specialized MovementComponents. Currently only saves pointers to pawn and input system.
 * Might be used for common replication later on.
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UMovementComponentBase : public UActorComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Already sets up VRPawn and InputSubsystem properties that can be used by child classes.
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem;
};
