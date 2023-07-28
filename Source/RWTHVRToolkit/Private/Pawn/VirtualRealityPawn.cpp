// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/UniversalTrackedComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Pawn/VRPawnMovement.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	BaseEyeHeight = 160.0f;
	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); //so it is rendered correctly in editor
	
	Head = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Head"));
	Head->ProxyType = ETrackedComponentType::TCT_HEAD;
	Head->SetupAttachment(RootComponent);

	CapsuleRotationFix = CreateDefaultSubobject<USceneComponent>(TEXT("CapsuleRotationFix"));
	CapsuleRotationFix->SetUsingAbsoluteRotation(true);
	CapsuleRotationFix->SetupAttachment(Head);

	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(CapsuleRotationFix);
	
	RightHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Right Hand"));
	RightHand->ProxyType = ETrackedComponentType::TCT_RIGHT_HAND;
	RightHand->AttachementType = EAttachementType::AT_FLYSTICK;
	RightHand->SetupAttachment(RootComponent);

	auto MCRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MC Right"));
	MCRight->SetTrackingSource(EControllerHand::Right);
	
	LeftHand = CreateDefaultSubobject<UUniversalTrackedComponent>(TEXT("Left Hand"));
	LeftHand->ProxyType = ETrackedComponentType::TCT_LEFT_HAND;
	LeftHand->AttachementType = EAttachementType::AT_HANDTARGET;
	LeftHand->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);
	
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!InputSubsystem)
	{
		UE_LOG(LogTemp,Error,TEXT("InputSubsystem IS NOT VALID"));
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCBase,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// old function bindings for grabbing and releasing
	EI->BindAction(InputActions->Fire, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginFire);
	EI->BindAction(InputActions->Fire, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndFire);

	EI->BindAction(InputActions->ToggleNavigationMode,ETriggerEvent::Started,this,&AVirtualRealityPawn::ToggleNavigationMode);

	// grabbing
	EI->BindAction(InputActions->Grab, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginGrab);
	EI->BindAction(InputActions->Grab, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndGrab);
	
}

// legacy grabbing
void AVirtualRealityPawn::OnBeginFire(const FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("BeginFire"));
	BasicVRInteraction->BeginInteraction();
}

// legacy grabbing
void AVirtualRealityPawn::OnEndFire(const FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("EndFire"));
	BasicVRInteraction->EndInteraction();
}

void AVirtualRealityPawn::OnBeginGrab(const FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("BeginGrab"));
}

void AVirtualRealityPawn::OnEndGrab(const FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("EndGrab"));
}

void AVirtualRealityPawn::ToggleNavigationMode(const FInputActionValue& Value)
{
	
	UE_LOG(LogTemp,Warning,TEXT("Toggle nav mode"));
	switch (PawnMovement->NavigationMode)
	{
		case EVRNavigationModes::NAV_FLY:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_WALK;
			break;

		case EVRNavigationModes::NAV_WALK:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_FLY;
			break;
		default:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_WALK;
	}
}


