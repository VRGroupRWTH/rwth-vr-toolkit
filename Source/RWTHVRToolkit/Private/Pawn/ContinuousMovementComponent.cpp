// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ContinuousMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Utility/VirtualRealityUtilities.h"

void UContinuousMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	SetupInputActions();
}
	

void UContinuousMovementComponent::SetupInputActions()
{
	Super::SetupInputActions();

	const AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	// simple way of changing the handedness
	if(bMoveWithRightHand)
	{
		MovementHand = VRPawn->RightHand;
		RotationHand = VRPawn->LeftHand;
		IMCMovement = IMCMovementRight;
	} else
	{
		MovementHand = VRPawn->LeftHand;
		RotationHand = VRPawn->RightHand;
		IMCMovement = IMCMovementLeft;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(VRPawn->GetController());
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!InputSubsystem)
	{
		UE_LOG(Toolkit,Error,TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(VRPawn->InputComponent);
	if(!EI)
	{
		UE_LOG(Toolkit,Error,TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}
	
	// walking
	EI->BindAction(Move, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnBeginMove);

	// turning is defined in MovementComponentBase
}

void UContinuousMovementComponent::OnBeginMove(const FInputActionValue& Value)
{
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	const bool bGazeDirected = UVirtualRealityUtilities::IsDesktopMode() || SteeringMode == EVRSteeringModes::STEER_GAZE_DIRECTED;
	
	const FVector ForwardDir = bGazeDirected ? VRPawn->Head->GetForwardVector() : MovementHand->GetForwardVector();
	const FVector RightDir = bGazeDirected ? VRPawn->Head->GetRightVector() : MovementHand->GetRightVector();
	
	if (VRPawn->Controller != nullptr)
	{
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
}

void UContinuousMovementComponent::OnBeginUp(const FInputActionValue& Value)
{
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	const float MoveValue =  Value.Get<FVector2D>().X;
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	VRPawn->AddMovementInput(FVector::UpVector, MoveValue);
}
