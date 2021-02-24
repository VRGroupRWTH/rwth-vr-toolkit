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
	CameraComponent->SetAbsolute();

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
	PlayerInputComponent->BindAxis("TurnRate", this, &AVirtualRealityPawn::OnTurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVirtualRealityPawn::OnLookUpRate);

	// function bindings for grabbing and releasing
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVirtualRealityPawn::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AVirtualRealityPawn::OnEndFire);
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	if (RightHand)
	{
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}
}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	if (RightHand)
	{
		AddMovementInput(RightHand->GetRightVector(), Value);
	}
}

void AVirtualRealityPawn::OnTurnRate_Implementation(float Rate)
{
	/* Turning the user externally will make them sick */
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	}
}

void AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate)
{
	/* Turning the user externally will make them sick */
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
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
