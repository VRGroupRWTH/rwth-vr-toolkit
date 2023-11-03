// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "MovementComponentBase.generated.h"

class AVirtualRealityPawn;
/**
 * 
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UMovementComponentBase : public UActorComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem;
};
