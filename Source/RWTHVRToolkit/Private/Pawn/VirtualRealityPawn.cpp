// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Pawn/ContinuousMovementComponent.h"
#include "Pawn/ReplicatedCameraComponent.h"
#include "Pawn/ReplicatedMotionControllerComponent.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Pawn/VRPawnMovement.h"
#include "Utility/VirtualRealityUtilities.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	BaseEyeHeight = 160.0f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));
	
	HeadCameraComponent = CreateDefaultSubobject<UReplicatedCameraComponent>(TEXT("Camera"));
	HeadCameraComponent->SetupAttachment(RootComponent);
	HeadCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); //so it is rendered correctly in editor
	
	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(HeadCameraComponent);
	
	RightHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Right Hand MCC"));
	RightHand->SetupAttachment(RootComponent);

	if(UVirtualRealityUtilities::IsDesktopMode())
	{
		RightHand->SetEnableGravity(false);
	 	RightHand->SetRelativeLocation(FVector(30,15,BaseEyeHeight-20));
		bUseControllerRotationYaw = true;
	}
	
	LeftHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);	
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
	// Set the control rotation of the PC to zero again. There is a small period of 2 frames where, when the pawn gets possessed,
	// the PC takes on the rotation of the VR Headset ONLY WHEN SPAWNING ON A CLIENT. Reset the rotation here such that
	// bUseControllerRotationYaw=true does not pass the wrong yaw value to the pawn initially.
	// There is probably a checkbox or way of spawning that prevents that in a better way that this, change if found.
	PlayerController->SetControlRotation(FRotator::ZeroRotator);
	
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
