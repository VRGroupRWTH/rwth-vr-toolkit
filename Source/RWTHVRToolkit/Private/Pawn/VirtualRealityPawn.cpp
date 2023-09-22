// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Pawn/ContinuousMovementComponent.h"
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


	if(UVirtualRealityUtilities::IsDesktopMode())
	{
		RightHand->SetEnableGravity(false);
		RightHand->SetRelativeLocation(FVector(30,15,BaseEyeHeight-20));
	}
	
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);	
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const ULocalPlayer* LP = PlayerController->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if(!InputSubsystem)
	{
		UE_LOG(Toolkit,Error,TEXT("[VirtualRealiytPawn.cpp] InputSubsystem IS NOT VALID"));
		return;
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCBase,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// old function bindings for grabbing and releasing
	EI->BindAction(Fire, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginFire);
	EI->BindAction(Fire, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndFire);

	EI->BindAction(ToggleNavigationMode,ETriggerEvent::Started,this,&AVirtualRealityPawn::OnToggleNavigationMode);

	// Set up mappings on movement components, need to do this nicely
	
	for (UActorComponent* Comp : GetComponentsByInterface(UMovementExtensionInterface::StaticClass()))
	{
		Cast<IMovementExtensionInterface>(Comp)->SetupPlayerInput(PlayerInputComponent);
	}
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


