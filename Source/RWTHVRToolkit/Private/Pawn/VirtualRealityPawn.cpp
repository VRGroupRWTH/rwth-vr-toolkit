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
#include "Utility/VirtualRealityUtilities.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	BaseEyeHeight = 160.0f;
	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));
	
	HeadCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	HeadCameraComponent->SetupAttachment(RootComponent);
	HeadCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); //so it is rendered correctly in editor
	
	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(HeadCameraComponent);
	
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Hand MCC"));
	RightHand->SetupAttachment(RootComponent);
	
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Hand MCC"));
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
		UE_LOG(Toolkit,Error,TEXT("[VirtualRealiytPawn.cpp] InputSubsystem IS NOT VALID"));
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCBase,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// old function bindings for grabbing and releasing
	EI->BindAction(Fire, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginFire);
	EI->BindAction(Fire, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndFire);

	EI->BindAction(ToggleNavigationMode,ETriggerEvent::Started,this,&AVirtualRealityPawn::OnToggleNavigationMode);	
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
	UE_LOG(Toolkit,Log,TEXT("EndFire"));
	BasicVRInteraction->EndInteraction();
}


void AVirtualRealityPawn::OnToggleNavigationMode(const FInputActionValue& Value)
{
	switch (PawnMovement->NavigationMode)
	{
		case EVRNavigationModes::NAV_FLY:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_WALK;
			UE_LOG(Toolkit,Log,TEXT("Changed Nav mode to WALK"));
			break;

		case EVRNavigationModes::NAV_WALK:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_FLY;
			UE_LOG(Toolkit,Log,TEXT("Changed Nav mode to FLY"));
			break;
		default:
			PawnMovement->NavigationMode = EVRNavigationModes::NAV_WALK;
			UE_LOG(Toolkit,Log,TEXT("Changed Nav mode to WALK"));
			break;
	}
}


