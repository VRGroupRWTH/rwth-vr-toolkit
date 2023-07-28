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

	VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	SetupInputActions();
	
}

void UContinuousMovementComponent::SetupInputActions()
{
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
		UE_LOG(LogTemp,Error,TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(VRPawn->InputComponent);
	if(!EI)
	{
		UE_LOG(LogTemp,Error,TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}
	
	// walking
	EI->BindAction(InputActions->Move, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnBeginMove);

	// turning
	if(bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Started, this, &UContinuousMovementComponent::OnBeginSnapTurn);
	} else
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnBeginTurn);
	}
	
	// bind functions for desktop rotations only on holding down right mouse
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
		if (PC)
		{
			PC->bShowMouseCursor = true; 
			PC->bEnableClickEvents = true; 
			PC->bEnableMouseOverEvents = true;
		} else
		{
			UE_LOG(LogTemp,Error,TEXT("PC Player Controller is invalid"));
		}
		EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Started, this, &UContinuousMovementComponent::StartDesktopRotation);
		EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Completed, this, &UContinuousMovementComponent::EndDesktopRotation);
		EI->BindAction(InputActions->MoveUp, ETriggerEvent::Triggered,this,&UContinuousMovementComponent::OnBeginUp);
	}
}

void UContinuousMovementComponent::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void UContinuousMovementComponent::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}

void UContinuousMovementComponent::OnBeginMove(const FInputActionValue& Value)
{
	
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

void UContinuousMovementComponent::OnBeginTurn(const FInputActionValue& Value)
{
	if(UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation) return;
	if (VRPawn->Controller != nullptr)
	{
		const FVector2D TurnValue = Value.Get<FVector2D>();
 
		if (TurnValue.X != 0.f)
		{
			VRPawn->AddControllerYawInput(TurnRateFactor * TurnValue.X);
			if (UVirtualRealityUtilities::IsDesktopMode())
			{
				UpdateRightHandForDesktopInteraction();
			}
		}
 
		if (TurnValue.Y != 0.f)
		{
			if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
			{
				VRPawn->AddControllerPitchInput(TurnRateFactor * -TurnValue.Y);
				SetCameraOffset();
			}
		}
	}
}

void UContinuousMovementComponent::OnBeginSnapTurn(const FInputActionValue& Value)
{
	const FVector2D TurnValue = Value.Get<FVector2D>();
	if (TurnValue.X != 0.f)
	{
		VRPawn->AddControllerYawInput(SnapTurnAngle);
	}
}

void UContinuousMovementComponent::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	VRPawn->GetActorEyesViewPoint(Location, Rotation);
	VRPawn->CameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}

void UContinuousMovementComponent::UpdateRightHandForDesktopInteraction()
{
	APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		VRPawn->RightHand->SetWorldRotation(HandOrientation);
	}
}

void UContinuousMovementComponent::OnBeginUp(const FInputActionValue& Value)
{
	const float MoveValue =  Value.Get<FVector2D>().X;
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	VRPawn->AddMovementInput(VRPawn->Head->GetUpVector(), MoveValue);
}
