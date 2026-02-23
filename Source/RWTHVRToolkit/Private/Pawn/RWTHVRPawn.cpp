// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/RWTHVRPawn.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ILiveLinkClient.h"
#include "InputMappingContext.h"
#include "Logging/StructuredLog.h"
#include "Pawn/InputExtensionInterface.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"
#include "Pawn/ReplicatedCameraComponent.h"
#include "Pawn/ReplicatedMotionControllerComponent.h"
#include "Utility/RWTHVRUtilities.h"
#include "Pawn/ClusterSetupComponent.h"
#include "Pawn/LiveLinkTrackingComponent.h"


ARWTHVRPawn::ARWTHVRPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BaseEyeHeight = 160.0f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));

	HeadCameraComponent = CreateDefaultSubobject<UReplicatedCameraComponent>(TEXT("Camera"));
	HeadCameraComponent->SetupAttachment(RootComponent);
	HeadCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	// so it is rendered correctly in editor

	CollisionHandlingMovement = CreateDefaultSubobject<UCollisionHandlingMovement>(TEXT("Collision Handling Movement"));
	CollisionHandlingMovement->SetUpdatedComponent(RootComponent);
	CollisionHandlingMovement->SetHeadComponent(HeadCameraComponent);

	ClusterSetupComponent = CreateDefaultSubobject<UClusterSetupComponent>(TEXT("ClusterSetupComponent"));
	LiveLinkTrackingComponent = CreateDefaultSubobject<ULiveLinkTrackingComponent>(TEXT("ClusterSetupComponent"));

	RightHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Right Hand MCC"));
	RightHand->SetupAttachment(RootComponent);

	LeftHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);

	UniformScale = GetActorScale3D().X;
	GetRootComponent()->TransformUpdated.AddLambda(
		[this](USceneComponent*, EUpdateTransformFlags, ETeleportType)
		{
			FVector CurrentScale = this->GetActorScale3D();
			if (CurrentScale.X != UniformScale || CurrentScale.Y != UniformScale || CurrentScale.Z != UniformScale)
			{
				UE_LOGFMT(Toolkit, Warning,
						  "ARWTHVRPawn: Do not adjust the scale of the pawn directly. This will not work in VR. Use "
						  "ARWTHVRPawn::SetScale(float) instead.");
			}
		});
}

void ARWTHVRPawn::BeginPlay()
{
	Super::BeginPlay();
	InitialWorldToMeters = GetWorldSettings()->WorldToMeters;
}

void ARWTHVRPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (URWTHVRUtilities::IsDesktopMode() && IsLocallyControlled())
	{
		SetCameraOffset();
		UpdateRightHandForDesktopInteraction();
	}
}

/*
 *	Scales the Pawn while also adjusting the WorldToMeters ratio to adjust for pupillary distance.
 *	Only supports uniform scaling.
 */
void ARWTHVRPawn::SetScale(float NewScale)
{
	FVector OldScale = GetActorScale();
	UniformScale = NewScale;
	FVector NewScaleVector = FVector(UniformScale, UniformScale, UniformScale);
	GetWorldSettings()->WorldToMeters = InitialWorldToMeters * UniformScale;
	SetActorRelativeScale3D(NewScaleVector);
	OnScaleChanged.Broadcast(OldScale, NewScale);
}

float ARWTHVRPawn::GetScale() { return UniformScale; }

void ARWTHVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	ActivePlayerInputComponent = PlayerInputComponent;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(Toolkit, Warning, TEXT("SetupPlayerInputComponent: Player Controller is invalid"));
		return;
	}

	UE_LOGFMT(Toolkit, Display, "SetupPlayerInputComponent: Player Controller is valid, setting up input for {Pawn}",
			  GetName());


	// Set the control rotation of the PC to zero again. There is a small period of 2 frames where, when the pawn gets
	// possessed, the PC takes on the rotation of the VR Headset ONLY WHEN SPAWNING ON A CLIENT. Reset the rotation here
	// such that bUseControllerRotationYaw=true does not pass the wrong yaw value to the pawn initially. There is
	// probably a checkbox or way of spawning that prevents that in a better way that this, change if found.
	PlayerController->SetControlRotation(FRotator::ZeroRotator);

	SetupMotionControllerSources();

	LiveLinkTrackingComponent->TrackedComponent = HeadCameraComponent;
	LiveLinkTrackingComponent->SubjectRepresentation = HeadSubjectRepresentation;

	// Should not do this here but on connection or on possess I think.

	if (URWTHVRUtilities::IsDesktopMode())
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
	}

	// Set up mappings on input extension components, need to do this nicely

	for (UActorComponent* Comp : GetComponentsByInterface(UInputExtensionInterface::StaticClass()))
	{
		Cast<IInputExtensionInterface>(Comp)->SetupPlayerInput(PlayerInputComponent);
	}

	// bind the current mapping contexts
	for (const auto Mapping : InputMappingContexts)
	{
		if (Mapping && IsValid(Mapping))
		{
			AddInputMappingContext(PlayerController, Mapping);
		}
		else
		{
			UE_LOGFMT(Toolkit, Warning, "ARWTHVRPawn::SetupPlayerInputComponent: InputMappingContext was invalid!");
		}
	}
}

UInputComponent* ARWTHVRPawn::GetPlayerInputComponent() { return ActivePlayerInputComponent; }


void ARWTHVRPawn::AddInputMappingContext(const APlayerController* PC, const UInputMappingContext* Context) const
{
	if (Context)
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				InputSub->AddMappingContext(Context, 0);
			}
			else
			{
				UE_LOGFMT(Toolkit, Warning,
						  "ARWTHVRPawn::AddInputMappingContext: UEnhancedInputLocalPlayerSubsystem is nullptr!");
			}
		}
		else
		{
			UE_LOGFMT(Toolkit, Warning, "ARWTHVRPawn::AddInputMappingContext: LocalPlayer is nullptr!");
		}
	}
	else
	{
		UE_LOGFMT(Toolkit, Warning, "ARWTHVRPawn::AddInputMappingContext: Context is nullptr!");
	}
}

void ARWTHVRPawn::UpdateRightHandForDesktopInteraction() const
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		const FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		if (bMoveRightHandWithMouse)
		{
			RightHand->SetWorldRotation(HandOrientation);
			RightHand->SetRelativeLocation(HeadCameraComponent->GetRelativeLocation());
		}
		if (bMoveLeftHandWithMouse)
		{
			LeftHand->SetWorldRotation(HandOrientation);
			LeftHand->SetRelativeLocation(HeadCameraComponent->GetRelativeLocation());
		}
	}
}

void ARWTHVRPawn::SetupMotionControllerSources()
{
	// Setup Motion Controllers

	FName MotionControllerSourceLeft = EName::None;
	FName MotionControllerSourceRight = EName::None;
	if (URWTHVRUtilities::IsHeadMountedMode())
	{
		MotionControllerSourceLeft = FName("Left");
		MotionControllerSourceRight = FName("Right");
	}
	if (URWTHVRUtilities::IsRoomMountedMode())
	{
		MotionControllerSourceLeft = LeftSubjectRepresentation.Subject;
		MotionControllerSourceRight = RightSubjectRepresentation.Subject;
	}
	LeftHand->SetTrackingMotionSource(MotionControllerSourceLeft);
	RightHand->SetTrackingMotionSource(MotionControllerSourceRight);
}

void ARWTHVRPawn::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);
	HeadCameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}
