// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/Navigation/MovementComponentBase.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/RWTHVRPawn.h"

void UMovementComponentBase::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	VRPawn = Cast<ARWTHVRPawn>(GetOwner());
}
