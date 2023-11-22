// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/Navigation/MovementComponentBase.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/RWTHVRPawn.h"
#include "Utility/RWTHVRUtilities.h"

void UMovementComponentBase::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	VRPawn = Cast<ARWTHVRPawn>(GetOwner());

	if (!VRPawn || !VRPawn->HasLocalNetOwner())
	{
		return;
	}

	InputSubsystem = GetEnhancedInputLocalPlayerSubsystem(VRPawn);
	if (!InputSubsystem)
	{
		UE_LOG(Toolkit, Error, TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
}
