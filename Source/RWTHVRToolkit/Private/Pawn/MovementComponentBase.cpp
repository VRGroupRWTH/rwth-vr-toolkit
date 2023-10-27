// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/MovementComponentBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Utility/VirtualRealityUtilities.h"

void UMovementComponentBase::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	VRPawn = Cast<AVirtualRealityPawn>(GetOwner());

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
	
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCRotation, 0);

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
			EI->BindAction(Turn, ETriggerEvent::Started, this, &UMovementComponentBase::OnBeginSnapTurn);
		}
		else
		{
			EI->BindAction(Turn, ETriggerEvent::Triggered, this, &UMovementComponentBase::OnBeginTurn);
		}
	}

	// bind additional functions for desktop rotations
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(DesktopRotation, ETriggerEvent::Started, this, &UMovementComponentBase::StartDesktopRotation);
		EI->BindAction(DesktopRotation, ETriggerEvent::Completed, this, &UMovementComponentBase::EndDesktopRotation);
	}
}

void UMovementComponentBase::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void UMovementComponentBase::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}

void UMovementComponentBase::OnBeginTurn(const FInputActionValue& Value)
{
	if (UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation)
		return;

	if (!VRPawn || !VRPawn->Controller)
		return;
	
	const FVector2D TurnValue = Value.Get<FVector2D>();

	if (TurnValue.X != 0.f)
	{
		VRPawn->AddControllerYawInput(TurnRateFactor * TurnValue.X);
	}

	if (TurnValue.Y != 0.f)
	{
		if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
		{
			VRPawn->AddControllerPitchInput(TurnRateFactor * -TurnValue.Y);
		}
	}
}

void UMovementComponentBase::OnBeginSnapTurn(const FInputActionValue& Value)
{
	if (!VRPawn || !VRPawn->Controller)
		return;
	
	const FVector2D TurnValue = Value.Get<FVector2D>();
	if (TurnValue.X > 0.f)
	{
		VRPawn->AddControllerYawInput(SnapTurnAngle);
	}
	else if (TurnValue.X < 0.f)
	{
		VRPawn->AddControllerYawInput(-SnapTurnAngle);
	}
}
