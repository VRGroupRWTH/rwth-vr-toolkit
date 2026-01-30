// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "Pawn/RWTHVRPawn.h"
#include "MovementComponentBase.generated.h"

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
	UFUNCTION(BlueprintCallable)
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void ChangeActorToMove(AActor* NewActorToMove)
	{
		ActorToMove = NewActorToMove;
	}
	
	UFUNCTION()
	virtual void ResetActorToMove()
	{
		ActorToMove = VRPawn;		
	}

protected:
	UPROPERTY()
	ARWTHVRPawn* VRPawn;
	
	// NEED TO DO SOME OWNER CHANGE HERE TO BE ABLE TO MOVE THIS ONE
	UPROPERTY()
	AActor* ActorToMove;	
	
	UPROPERTY()
	USceneComponent* ReferenceComponent;
};
