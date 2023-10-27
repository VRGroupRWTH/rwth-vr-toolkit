// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/ContinuousMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Utility/VirtualRealityUtilities.h"
#include "MotionControllerComponent.h"

void UContinuousMovementComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	VRPawn = Cast<AVirtualRealityPawn>(GetOwner());

	if (!VRPawn || !VRPawn->HasLocalNetOwner())
	{
		return;
	}

	// simple way of changing the handedness
	if (bMoveWithRightHand)
	{
		MovementHand = VRPawn->RightHand;
		RotationHand = VRPawn->LeftHand;
		IMCMovement = IMCMovementRight;
	}
	else
	{
		MovementHand = VRPawn->LeftHand;
		RotationHand = VRPawn->RightHand;
		IMCMovement = IMCMovementLeft;
	}

	auto* InputSubsystem = GetEnhancedInputLocalPlayerSubsystem(VRPawn);
	if (!InputSubsystem)
	{
		UE_LOG(Toolkit, Error, TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement, 0);

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EI)
	{
		UE_LOG(Toolkit, Error, TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}

	// continuous steering
	EI->BindAction(Move, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnMove);
	EI->BindAction(MoveUp, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnMoveUp);

	// turning is defined in MovementComponentBase
}

void UContinuousMovementComponent::OnMove(const FInputActionValue& Value)
{
	if (!VRPawn || !VRPawn->Controller)
		return;

	const bool bGazeDirected = UVirtualRealityUtilities::IsDesktopMode() || SteeringMode ==
		EVRSteeringModes::STEER_GAZE_DIRECTED;

	const FVector ForwardDir = bGazeDirected
		                           ? VRPawn->HeadCameraComponent->GetForwardVector()
		                           : MovementHand->GetForwardVector();
	const FVector RightDir = bGazeDirected
		                         ? VRPawn->HeadCameraComponent->GetRightVector()
		                         : MovementHand->GetRightVector();

	const FVector2D MoveValue = Value.Get<FVector2D>();

	// Forward/Backward direction
	if (MoveValue.X != 0.f)
	{
		VRPawn->AddMovementInput(ForwardDir, MoveValue.X);
	}

	// Right/Left direction
	if (MoveValue.Y != 0.f)
	{
		VRPawn->AddMovementInput(RightDir, MoveValue.Y);
	}
}

void UContinuousMovementComponent::OnMoveUp(const FInputActionValue& Value)
{
	if (!VRPawn)
		return;

	const float MoveValue = Value.Get<FVector2D>().X;
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	VRPawn->AddMovementInput(FVector::UpVector, MoveValue);
}
