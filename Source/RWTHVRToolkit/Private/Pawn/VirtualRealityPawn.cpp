// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Core/RWTHVRPlayerState.h"
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
	
	LeftHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);

	BasicVRInteraction = CreateDefaultSubobject<UBasicVRInteractionComponent>(TEXT("Basic VR Interaction"));
	BasicVRInteraction->Initialize(RightHand);
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		SetCameraOffset();
		UpdateRightHandForDesktopInteraction();
	}
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) 
	{
		UE_LOG(Toolkit, Warning, TEXT("SetupPlayerInputComponent: Player Controller is invalid"));
		return;
	}
	// Set the control rotation of the PC to zero again. There is a small period of 2 frames where, when the pawn gets possessed,
	// the PC takes on the rotation of the VR Headset ONLY WHEN SPAWNING ON A CLIENT. Reset the rotation here such that
	// bUseControllerRotationYaw=true does not pass the wrong yaw value to the pawn initially.
	// There is probably a checkbox or way of spawning that prevents that in a better way that this, change if found.
	PlayerController->SetControlRotation(FRotator::ZeroRotator);
	
	const ULocalPlayer* LP = PlayerController->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if(!InputSubsystem)
	{
		UE_LOG(Toolkit, Error, TEXT("[VirtualRealiytPawn.cpp] InputSubsystem IS NOT VALID"));
		return;
	}

	ARWTHVRPlayerState* State = GetPlayerState<ARWTHVRPlayerState>();

	// Should not do this here but on connection or on possess I think.
	if (State)
	{
		// Might not be properly synced yet?
		const EPlayerType Type = State->GetPlayerType();

		// Don't do anything with the type if it's been set to clustertype or anything.
		const bool bClusterType = Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary;

		if (!bClusterType && UVirtualRealityUtilities::IsHeadMountedMode())
		{
			// Could be too early to call this RPC...
			State->RequestSetPlayerType(Type);
		}
	}
	
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCBase,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// old function bindings for grabbing and releasing
	EI->BindAction(Fire, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginFire);
	EI->BindAction(Fire, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndFire);

	EI->BindAction(ToggleNavigationMode,ETriggerEvent::Started,this,&AVirtualRealityPawn::OnToggleNavigationMode);

	// Set up mappings on input extension components, need to do this nicely
	
	for (UActorComponent* Comp : GetComponentsByInterface(UInputExtensionInterface::StaticClass()))
	{
		Cast<IInputExtensionInterface>(Comp)->SetupPlayerInput(PlayerInputComponent);
	}
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
		RightHand->SetRelativeLocation(HeadCameraComponent->GetRelativeLocation());
	}
}

void AVirtualRealityPawn::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);
	HeadCameraComponent->SetWorldLocationAndRotation(Location, Rotation);
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
