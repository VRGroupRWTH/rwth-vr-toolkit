// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "AITypes.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/UniversalTrackedComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Pawn/VRPawnInputConfig.h"

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
	
	/*PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(CapsuleRotationFix);*/
	
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
	
	// simple way of changing the handedness
	/*if(bMoveWithRightHand)
	{
		MovementHand = RightHand;
		RotationHand = LeftHand;
		IMCMovement = IMCMovementRight;
	} else
	{
		MovementHand = LeftHand;
		RotationHand = RightHand;
		IMCMovement = IMCMovementLeft;
	}*/
	
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!InputSubsystem)
	{
		UE_LOG(LogTemp,Error,TEXT("InputSubsystem IS NOT VALID"));
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	//InputSubsystem->AddMappingContext(IMCMovement,0);
	InputSubsystem->AddMappingContext(IMCBase,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// old function bindings for grabbing and releasing
	EI->BindAction(InputActions->Fire, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginFire);
	EI->BindAction(InputActions->Fire, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndFire);

	// grabbing
	EI->BindAction(InputActions->Grab, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginGrab);
	EI->BindAction(InputActions->Grab, ETriggerEvent::Completed, this, &AVirtualRealityPawn::OnEndGrab);

	/*// walking
	EI->BindAction(InputActions->Move, ETriggerEvent::Triggered, this, &AVirtualRealityPawn::OnBeginMove);*/

	/*
	// turning
	if(bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Started, this, &AVirtualRealityPawn::OnBeginSnapTurn);
	} else
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Triggered, this, &AVirtualRealityPawn::OnBeginTurn);
	}
	*/
	
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
		/*EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Started, this, &AVirtualRealityPawn::StartDesktopRotation);
		EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Completed, this, &AVirtualRealityPawn::EndDesktopRotation);*/
	}
}

/*void AVirtualRealityPawn::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void AVirtualRealityPawn::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}*/

/*void AVirtualRealityPawn::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);
	CameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}*/

/*void AVirtualRealityPawn::UpdateRightHandForDesktopInteraction()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		RightHand->SetWorldRotation(HandOrientation);
	}
}*/

/*void AVirtualRealityPawn::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AVirtualRealityPawn, bMoveWithRightHand) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AVirtualRealityPawn, bSnapTurn))
	{
		// if we want to change input bindings, we need to setup the player input again to load a different mapping context,
		// or assign a different function to an input action.
		// This is automatically done by restarting the pawn(calling SetupPlayerInputComponent() directly results in crashes)
		// only do this in the editor
		#if WITH_EDITOR
			Restart();
		#endif
	}
}*/


void AVirtualRealityPawn::OnUp(const FInputActionValue& Value)
{
	const float MoveValue =  Value.Get<FVector2D>().X;
	UE_LOG(LogTemp,Warning,TEXT("MoveUp: %f"),MoveValue);
	//the right hand is rotated on desktop to follow the cursor so it's forward is also changing with cursor position
	if (RightHand && !UVirtualRealityUtilities::IsDesktopMode())
	{
		AddMovementInput(RightHand->GetUpVector(), MoveValue);
	}
	else if (Head)
	{
		AddMovementInput(Head->GetUpVector(), MoveValue);
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

/*void AVirtualRealityPawn::OnBeginMove(const FInputActionValue& Value)
{
	const FVector ForwardDir = UVirtualRealityUtilities::IsDesktopMode() ? Head->GetForwardVector() : MovementHand->GetForwardVector();
	const FVector RightDir = UVirtualRealityUtilities::IsDesktopMode() ? Head->GetRightVector() : MovementHand->GetRightVector();
	
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);
 
		// Forward/Backward direction
		if (MoveValue.X != 0.f)
		{
			AddMovementInput(ForwardDir, MoveValue.X);
		}
 
		// Right/Left direction
		if (MoveValue.Y != 0.f)
		{
			AddMovementInput(RightDir, MoveValue.Y);
		}
	}
}*/

/*void AVirtualRealityPawn::OnBeginTurn(const FInputActionValue& Value)
{
	if(UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation) return;
	if (Controller != nullptr)
	{
		const FVector2D TurnValue = Value.Get<FVector2D>();
 
		if (TurnValue.X != 0.f)
		{
			AddControllerYawInput(TurnRateFactor * TurnValue.X);
			if (UVirtualRealityUtilities::IsDesktopMode())
			{
				UpdateRightHandForDesktopInteraction();
			}
		}
 
		if (TurnValue.Y != 0.f)
		{
			if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
			{
				AddControllerPitchInput(TurnRateFactor * TurnValue.Y);
				SetCameraOffset();
			}
		}
	}
}

void AVirtualRealityPawn::OnBeginSnapTurn(const FInputActionValue& Value)
{
	const FVector2D TurnValue = Value.Get<FVector2D>();
 
	if (TurnValue.X != 0.f)
	{
		AddControllerYawInput(SnapTurnAngle);
	}
}*/


