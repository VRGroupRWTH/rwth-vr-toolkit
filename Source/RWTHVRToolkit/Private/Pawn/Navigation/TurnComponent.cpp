// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/Navigation/TurnComponent.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Pawn/RWTHVRPawn.h"
#include "Utility/RWTHVRUtilities.h"

void UTurnComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInput(PlayerInputComponent);

	if (!VRPawn || !VRPawn->HasLocalNetOwner())
	{
		return;
	}

	// simple way of changing the handedness
	if (bTurnWithLeftHand)
	{
		RotationHand = VRPawn->LeftHand;
	}
	else
	{
		RotationHand = VRPawn->RightHand;
	}

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EI)
	{
		UE_LOG(Toolkit, Error, TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}

	// turning
	if (bAllowTurning)
	{
		if (bSnapTurn)
		{
			// no snap turning for desktop mode
			if (!URWTHVRUtilities::IsDesktopMode())
			{
				EI->BindAction(Turn, ETriggerEvent::Started, this, &UTurnComponent::OnBeginSnapTurn);
			}
			else
			{
				EI->BindAction(DesktopTurn, ETriggerEvent::Triggered, this, &UTurnComponent::OnBeginTurn);
			}
		}
		else
		{
			if (!URWTHVRUtilities::IsDesktopMode())
			{
				EI->BindAction(Turn, ETriggerEvent::Triggered, this, &UTurnComponent::OnBeginTurn);
			}
			else
			{
				EI->BindAction(DesktopTurn, ETriggerEvent::Triggered, this, &UTurnComponent::OnBeginTurn);
			}
		}
	}

	// bind additional functions for desktop rotations
	if (URWTHVRUtilities::IsDesktopMode())
	{
		EI->BindAction(DesktopTurnCondition, ETriggerEvent::Started, this, &UTurnComponent::StartDesktopRotation);
		EI->BindAction(DesktopTurnCondition, ETriggerEvent::Completed, this, &UTurnComponent::EndDesktopRotation);
	}
}

// A separate button has to be pressed, for when mouse movement should contribute to turning
void UTurnComponent::StartDesktopRotation() { bApplyDesktopRotation = true; }

void UTurnComponent::EndDesktopRotation() { bApplyDesktopRotation = false; }

void UTurnComponent::OnBeginTurn(const FInputActionValue& Value)
{
	if (URWTHVRUtilities::IsDesktopMode() && !bApplyDesktopRotation)
		return;

	if (!VRPawn || !VRPawn->Controller)
		return;

	const FVector2D TurnValue = Value.Get<FVector2D>();

	if (TurnValue.X != 0.f)
	{
		if (URWTHVRUtilities::IsDesktopMode() && bApplyDesktopRotation)
		{
			VRPawn->AddControllerYawInput(TurnRateFactor * TurnValue.X);
		}
		else
		{
			RotateCameraAndPawn(TurnRateFactor * TurnValue.X);
		}
	}

	if (TurnValue.Y != 0.f)
	{
		if (URWTHVRUtilities::IsDesktopMode() && bApplyDesktopRotation)
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


void UTurnComponent::RotateCameraAndPawn(float Yaw) const
{
	const FVector OrigLocation = VRPawn->HeadCameraComponent->GetComponentLocation();

	const FRotator OrigRotation = VRPawn->GetActorRotation();
	const FRotator NewRotation = FRotator(0, VRPawn->GetActorRotation().Yaw + Yaw, 0);

	const FVector Offset = VRPawn->GetActorLocation() - OrigLocation;
	const FVector UntwistedOffset = OrigRotation.GetInverse().RotateVector(Offset);
	const FVector NewLocation = OrigLocation + NewRotation.RotateVector(UntwistedOffset);

	VRPawn->Controller->SetControlRotation(NewRotation);
	VRPawn->SetActorLocationAndRotation(NewLocation, NewRotation);
}
