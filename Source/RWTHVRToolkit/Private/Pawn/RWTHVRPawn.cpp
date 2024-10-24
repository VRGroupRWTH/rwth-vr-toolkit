// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/RWTHVRPawn.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ILiveLinkClient.h"
#include "InputMappingContext.h"
#include "Core/RWTHVRPlayerState.h"
#include "Logging/StructuredLog.h"
#include "Pawn/ClusterRepresentationActor.h"
#include "Pawn/InputExtensionInterface.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"
#include "Pawn/ReplicatedCameraComponent.h"
#include "Pawn/ReplicatedMotionControllerComponent.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "Utility/RWTHVRUtilities.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "Components/DisplayClusterSceneComponentSyncParent.h"
#endif

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

	RightHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Right Hand MCC"));
	RightHand->SetupAttachment(RootComponent);

	LeftHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);
}
void ARWTHVRPawn::BeginPlay() { Super::BeginPlay(); }

void ARWTHVRPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (URWTHVRUtilities::IsDesktopMode() && IsLocallyControlled())
	{
		SetCameraOffset();
		UpdateRightHandForDesktopInteraction();
	}
	EvaluateLivelink();
}

/*
 * The alternative would be to do this only on the server on possess and check for player state/type,
 * as connections now send their playertype over.
 */
// This pawn's controller has changed! This is called on both server and owning client. If we are the owning client
// and the master, request that the Cluster is attached to us.
void ARWTHVRPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Try and use PlayerType for this:

	if (HasAuthority())
	{
		UE_LOG(Toolkit, Display,
			   TEXT("ARWTHVRPawn: Player Controller has changed, trying to change Cluster attachment if possible..."));
		if (const ARWTHVRPlayerState* State = GetPlayerState<ARWTHVRPlayerState>())
		{
			const EPlayerType Type = State->GetPlayerType();

			// Only cluster types are valid here as they are set on connection.
			// For all other player types this is a race condition
			if (Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary)
			{
				UE_LOGFMT(Toolkit, Display, "ARWTHVRPawn: Attaching Cluster to Pawn {Pawn}.", GetName());
				AttachClustertoPawn();
			}
		}
	}
}

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

	// Should not do this here but on connection or on possess I think.
	if (ARWTHVRPlayerState* State = GetPlayerState<ARWTHVRPlayerState>())
	{
		// Might not be properly synced yet?
		const EPlayerType Type = State->GetPlayerType();

		// Don't do anything with the type if it's been set to clustertype or anything.
		// This is already being done when connecting to the server.
		const bool bClusterType = Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary;

		if (!bClusterType && URWTHVRUtilities::IsHeadMountedMode())
		{
			// Could be too early to call this RPC...
			State->RequestSetPlayerType(Type);
		}
	}

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

void ARWTHVRPawn::EvaluateLivelink() const
{
	if (URWTHVRUtilities::IsRoomMountedMode() && IsLocallyControlled())
	{
		if (bDisableLiveLink || HeadSubjectRepresentation.Subject.IsNone() || HeadSubjectRepresentation.Role == nullptr)
		{
			return;
		}

		// Get the LiveLink interface and evaluate the current existing frame data for the given Subject and Role.
		ILiveLinkClient& LiveLinkClient =
			IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		FLiveLinkSubjectFrameData SubjectData;
		const bool bHasValidData = LiveLinkClient.EvaluateFrame_AnyThread(HeadSubjectRepresentation.Subject,
																		  HeadSubjectRepresentation.Role, SubjectData);

		if (!bHasValidData)
		{
			return;
		}

		// Assume we are using a Transform Role to track the components! This is a slightly dangerous assumption, and
		// could be further improved.
		const FLiveLinkTransformStaticData* StaticData = SubjectData.StaticData.Cast<FLiveLinkTransformStaticData>();
		const FLiveLinkTransformFrameData* FrameData = SubjectData.FrameData.Cast<FLiveLinkTransformFrameData>();

		if (StaticData && FrameData)
		{
			// Finally, apply the transform to this component according to the static data.
			ApplyLiveLinkTransform(FrameData->Transform, *StaticData);
		}
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

void ARWTHVRPawn::MulticastAddDCSyncComponent_Implementation()
{
#if PLATFORM_SUPPORTS_CLUSTER
	// Add an nDisplay Parent Sync Component. It syncs the parent's transform from master to clients.
	// This is required because for collision based movement, it can happen that the physics engine
	// for some reason acts different on the nodes, therefore leading to a potential desync when
	// e.g. colliding with an object while moving.

	if (URWTHVRUtilities::IsRoomMountedMode())
	{
		SyncComponent = Cast<USceneComponent>(AddComponentByClass(
			UDisplayClusterSceneComponentSyncParent::StaticClass(), false, FTransform::Identity, false));
		AddInstanceComponent(SyncComponent);
		UE_LOGFMT(Toolkit, Display, "RWTHVRPawn: Added Sync Component to pawn {Pawn}", GetName());
	}
#endif
}

// Executed on the server only: Attaches the ClusterRepresentation Actor, which contains the DCRA to the Pawn.
// It is only executed on the server because attachments are synced to all clients, but not from client to server.
void ARWTHVRPawn::AttachClustertoPawn()
{
	if (const ARWTHVRPlayerState* State = GetPlayerState<ARWTHVRPlayerState>())
	{
		const auto ClusterActor = State->GetCorrespondingClusterActor();
		if (!ClusterActor)
		{
			UE_LOGFMT(
				Toolkit, Error,
				"ARWTHVRPawn::AttachClustertoPawn: GetCorrespondingClusterActor returned null! This won't work on "
				"the Cave.");
			return;
		}
		const FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		bool bAttached = ClusterActor->AttachToComponent(GetRootComponent(), AttachmentRules);
		// State->GetCorrespondingClusterActor()->OnAttached();
		UE_LOGFMT(Toolkit, Display,
				  "ARWTHVRPawn: Attaching corresponding cluster actor to our pawn returned: {Attached}", bAttached);
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "ARWTHVRPawn::AttachClustertoPawn: No ARWTHVRPlayerState set! This won't work on the Cave.");
	}

	if (HasAuthority()) // Should always be the case here, but double check
		MulticastAddDCSyncComponent();
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

void ARWTHVRPawn::ApplyLiveLinkTransform(const FTransform& Transform,
										 const FLiveLinkTransformStaticData& StaticData) const
{
	if (StaticData.bIsLocationSupported)
	{
		if (bWorldTransform)
		{
			HeadCameraComponent->SetWorldLocation(Transform.GetLocation(), false, nullptr,
												  ETeleportType::TeleportPhysics);
		}
		else
		{
			HeadCameraComponent->SetRelativeLocation(Transform.GetLocation(), false, nullptr,
													 ETeleportType::TeleportPhysics);
		}
	}

	if (StaticData.bIsRotationSupported)
	{
		if (bWorldTransform)
		{
			HeadCameraComponent->SetWorldRotation(Transform.GetRotation(), false, nullptr,
												  ETeleportType::TeleportPhysics);
		}
		else
		{
			HeadCameraComponent->SetRelativeRotation(Transform.GetRotation(), false, nullptr,
													 ETeleportType::TeleportPhysics);
		}
	}

	if (StaticData.bIsScaleSupported)
	{
		if (bWorldTransform)
		{
			HeadCameraComponent->SetWorldScale3D(Transform.GetScale3D());
		}
		else
		{
			HeadCameraComponent->SetRelativeScale3D(Transform.GetScale3D());
		}
	}
}
