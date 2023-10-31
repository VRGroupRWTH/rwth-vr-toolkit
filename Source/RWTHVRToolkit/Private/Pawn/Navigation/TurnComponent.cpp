// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/Navigation/TurnComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MotionControllerComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Utility/VirtualRealityUtilities.h"

void UTurnComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInput(PlayerInputComponent);
	
	if (!VRPawn || !VRPawn->HasLocalNetOwner() || !InputSubsystem)
	{
		return;
	}

	// simple way of changing the handedness
	if(bTurnWithLeftHand)
	{
		RotationHand = VRPawn->LeftHand;
		// we use the same IMC for movement and turning
		// therefore if we move with the right hand, we turn with the left hand
		IMCTurn = IMCMovement_Right;
	} else
	{
		RotationHand = VRPawn->RightHand;
		// we use the same IMC for movement and turning
		// therefore if we move with the left hand, we turn with the right hand
		IMCTurn = IMCMovement_Left;
	}

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCTurn, 0);
	InputSubsystem->AddMappingContext(IMCDesktopRotation, 0);

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EI)
	{
		UE_LOG(Toolkit, Error, TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}

	// turning
	if (bAllowTurning)
	{
		// no snap turning for desktop mode
		if (bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
		{
			EI->BindAction(Turn, ETriggerEvent::Started, this, &UTurnComponent::OnBeginSnapTurn);
		}
		else
		{
			EI->BindAction(Turn, ETriggerEvent::Triggered, this, &UTurnComponent::OnBeginTurn);
		}
	}

	// bind additional functions for desktop rotations
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(DesktopRotation, ETriggerEvent::Started, this, &UTurnComponent::StartDesktopRotation);
		EI->BindAction(DesktopRotation, ETriggerEvent::Completed, this, &UTurnComponent::EndDesktopRotation);
	}
}

void UTurnComponent::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void UTurnComponent::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}

void UTurnComponent::OnBeginTurn(const FInputActionValue& Value)
{
	if (UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation)
		return;

	if (!VRPawn || !VRPawn->Controller)
		return;

	const FVector2D TurnValue = Value.Get<FVector2D>();

	if (TurnValue.X != 0.f)
	{
		RotateCameraAndPawn(TurnRateFactor * TurnValue.X);
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
		}
	}
}

void UTurnComponent::OnBeginSnapTurn(const FInputActionValue& Value)
{
	if (!VRPawn || !VRPawn->Controller)
		return;

	const FVector2D TurnValue = Value.Get<FVector2D>();
	if (TurnValue.X > 0.f)
	{
		RotateCameraAndPawn(SnapTurnAngle);
	}
	else if (TurnValue.X < 0.f)
	{
		RotateCameraAndPawn(-SnapTurnAngle);
	}
}

void UTurnComponent::RotateCameraAndPawn(float Yaw)
{
	FVector NewLocation;
	FRotator NewRotation;
	
	FVector OrigLocation = VRPawn->GetActorLocation();
	FVector PivotPoint = VRPawn->GetActorTransform().InverseTransformPosition(OrigLocation);
	PivotPoint.Z = 0.0f;

	FRotator OrigRotation = VRPawn->GetActorRotation();

	NewRotation = FRotator(0,VRPawn->GetActorRotation().Yaw+Yaw,0);

	NewLocation = OrigLocation + OrigRotation.RotateVector(PivotPoint);

	VRPawn->Controller->SetControlRotation(NewRotation);
	VRPawn->SetActorLocationAndRotation(NewLocation,NewRotation);
	
	//FVector MovedBy = NewLocation - OrigLocation;
}
