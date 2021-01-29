// Fill out your copyright notice in the Description page of Project Settings.


#include "VirtualRealityPawn.h"



#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "UniversalTrackedComponent.h"
#include "VirtualRealityUtilities.h"
#include "VRPawnMovement.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	BaseEyeHeight = 160.0f;
	
	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->UpdatedComponent = RootComponent;

	Head = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Head"));
	Head->ProxyType = ETrackedComponentType::TCT_HEAD;
	Head->SetupAttachment(RootComponent);
	
	RightHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Right Hand"));
	RightHand->ProxyType = ETrackedComponentType::TCT_RIGHT_HAND;
	RightHand->AttachementType = EAttachementType::AT_FLYSTICK;
	RightHand->SetupAttachment(RootComponent);
	
	LeftHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Left Hand"));
	LeftHand->ProxyType = ETrackedComponentType::TCT_LEFT_HAND;
	LeftHand->AttachementType = EAttachementType::AT_HANDTARGET;
	LeftHand->SetupAttachment(RootComponent);

	Tracker1 = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Vive Tracker 1"));
	Tracker1->ProxyType = ETrackedComponentType::TCT_TRACKER_1;
	Tracker1->SetupAttachment(RootComponent);

	Tracker2 = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Vive Tracker 2"));
	Tracker2->ProxyType = ETrackedComponentType::TCT_TRACKER_2;
	Tracker2->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAxis("MoveForward", this, &AVirtualRealityPawn::OnForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AVirtualRealityPawn::OnRight);
		PlayerInputComponent->BindAxis("TurnRate", this, &AVirtualRealityPawn::OnTurnRate);
		PlayerInputComponent->BindAxis("LookUpRate", this, &AVirtualRealityPawn::OnLookUpRate);

		// function bindings for grabbing and releasing
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVirtualRealityPawn::OnBeginFire);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &AVirtualRealityPawn::OnEndFire);
	}
}

void AVirtualRealityPawn::BeginPlay()
{
	if(!UVirtualRealityUtilities::IsDesktopMode()) /* Disable to not get cyber sick as fast */
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));
	}
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
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate)
{
	// User-centered projection causes simulation sickness on look up interaction hence not implemented.
	if (!UVirtualRealityUtilities::IsRoomMountedMode())
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
