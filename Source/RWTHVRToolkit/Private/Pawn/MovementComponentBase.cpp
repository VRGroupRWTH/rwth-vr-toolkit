// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/MovementComponentBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Utility/VirtualRealityUtilities.h"

// Sets default values for this component's properties
UMovementComponentBase::UMovementComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UMovementComponentBase::BeginPlay()
{
	Super::BeginPlay();

	SetupInputActions();
}

// Called every frame
void UMovementComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		UpdateRightHandForDesktopInteraction();
	}
}
	

void UMovementComponentBase::SetupInputActions()
{

	const AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	
	const APlayerController* PlayerController = Cast<APlayerController>(VRPawn->GetController());
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!InputSubsystem)
	{
		UE_LOG(Toolkit,Error,TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCRotation,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(VRPawn->InputComponent);
	if(!EI)
	{
		UE_LOG(Toolkit,Error,TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}
	

	// turning
	if(bAllowTurning)
	{
		// no snap turning for desktop mode
		if(bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
		{
			EI->BindAction(Turn, ETriggerEvent::Started, this, &UMovementComponentBase::OnBeginSnapTurn);
		} else
		{
			EI->BindAction(Turn, ETriggerEvent::Triggered, this, &UMovementComponentBase::OnBeginTurn);
		}
	}
	
	// bind additional functions for desktop rotations
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		if (APlayerController* PC = Cast<APlayerController>(VRPawn->GetController()))
		{
			PC->bShowMouseCursor = true; 
			PC->bEnableClickEvents = true; 
			PC->bEnableMouseOverEvents = true;
		} else
		{
			UE_LOG(LogTemp,Error,TEXT("PC Player Controller is invalid"));
		}
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
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());

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

void UMovementComponentBase::OnBeginSnapTurn(const FInputActionValue& Value)
{
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	const FVector2D TurnValue = Value.Get<FVector2D>();
	if (TurnValue.X > 0.f)
	{
		VRPawn->AddControllerYawInput(SnapTurnAngle);
	} else if (TurnValue.X < 0.f)
	{
		VRPawn->AddControllerYawInput(-SnapTurnAngle);
	}
}

void UMovementComponentBase::SetCameraOffset() const
{
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	VRPawn->GetActorEyesViewPoint(Location, Rotation);
	VRPawn->CameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}

void UMovementComponentBase::UpdateRightHandForDesktopInteraction() const
{
	AVirtualRealityPawn* VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		const FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		VRPawn->RightHand->SetWorldRotation(HandOrientation);
	}
}
