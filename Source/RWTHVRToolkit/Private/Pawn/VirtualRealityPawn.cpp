// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/VirtualRealityPawn.h"



#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Pawn/UniversalTrackedComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "Pawn/VRPawnMovement.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	BaseEyeHeight = 160.0f;
	
	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); //so it is rendered correctly in editor
	
	Head = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Head"));
	Head->ProxyType = ETrackedComponentType::TCT_HEAD;
	Head->SetupAttachment(RootComponent);

	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(Head);
	
	RightHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Right Hand"));
	RightHand->ProxyType = ETrackedComponentType::TCT_RIGHT_HAND;
	RightHand->AttachementType = EAttachementType::AT_FLYSTICK;
	RightHand->SetupAttachment(RootComponent);
	
	LeftHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Left Hand"));
	LeftHand->ProxyType = ETrackedComponentType::TCT_LEFT_HAND;
	LeftHand->AttachementType = EAttachementType::AT_HANDTARGET;
	LeftHand->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AVirtualRealityPawn::OnForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVirtualRealityPawn::OnRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AVirtualRealityPawn::OnUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVirtualRealityPawn::OnTurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVirtualRealityPawn::OnLookUpRate);

	// function bindings for grabbing and releasing
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVirtualRealityPawn::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AVirtualRealityPawn::OnEndFire);

	// bind functions for desktop rotations only on holding down right mouse
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PC->bShowMouseCursor = true; 
			PC->bEnableClickEvents = true; 
			PC->bEnableMouseOverEvents = true;
		}
		PlayerInputComponent->BindAction("EnableDesktopRotation", IE_Pressed, this, &AVirtualRealityPawn::StartDesktopRotation);
		PlayerInputComponent->BindAction("EnableDesktopRotation", IE_Released, this, &AVirtualRealityPawn::EndDesktopRotation);
	}
}

void AVirtualRealityPawn::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void AVirtualRealityPawn::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}

void AVirtualRealityPawn::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);
	CameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}

void AVirtualRealityPawn::UpdateRightHandForDesktopInteraction()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		RightHand->SetWorldRotation(HandOrientation);
	}
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	if (RightHand && !UVirtualRealityUtilities::IsDesktopMode())
	{
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}
	else if (Head)
	{
		AddMovementInput(Head->GetForwardVector(), Value);
	}
}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	if (RightHand && !UVirtualRealityUtilities::IsDesktopMode())
	{
		AddMovementInput(RightHand->GetRightVector(), Value);
	}
	else if (Head)
	{
		AddMovementInput(Head->GetRightVector(), Value);
	}
}

void AVirtualRealityPawn::OnUp_Implementation(float Value)
{
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	if (RightHand && !UVirtualRealityUtilities::IsDesktopMode())
	{
		AddMovementInput(RightHand->GetUpVector(), Value);
	}
	else if (Head)
	{
		AddMovementInput(Head->GetUpVector(), Value);
	}
}

void AVirtualRealityPawn::OnTurnRate_Implementation(float Rate)
{
	/* Turning the user externally will make them sick */
	if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	}
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		UpdateRightHandForDesktopInteraction();
	}
}

void AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate)
{
	/* Turning the user externally will make them sick */
	if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
	{
		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
		SetCameraOffset();
	}
}

void AVirtualRealityPawn::OnBeginFire_Implementation()
{
	BasicVRInteraction->BeginInteraction();
}

void AVirtualRealityPawn::OnEndFire_Implementation()
{
	BasicVRInteraction->EndInteraction();
}
