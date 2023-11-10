// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VirtualRealityPawn.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ILiveLinkClient.h"
#include "Core/RWTHVRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Pawn/InputExtensionInterface.h"
#include "Pawn/Navigation/VRPawnMovement.h"
#include "Pawn/ReplicatedCameraComponent.h"
#include "Pawn/ReplicatedMotionControllerComponent.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "Utility/VirtualRealityUtilities.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BaseEyeHeight = 160.0f;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));

	HeadCameraComponent = CreateDefaultSubobject<UReplicatedCameraComponent>(TEXT("Camera"));
	HeadCameraComponent->SetupAttachment(RootComponent);
	HeadCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	//so it is rendered correctly in editor

	PawnMovement = CreateDefaultSubobject<UVRPawnMovement>(TEXT("Pawn Movement"));
	PawnMovement->SetUpdatedComponent(RootComponent);
	PawnMovement->SetHeadComponent(HeadCameraComponent);

	RightHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Right Hand MCC"));
	RightHand->SetupAttachment(RootComponent);

	LeftHand = CreateDefaultSubobject<UReplicatedMotionControllerComponent>(TEXT("Left Hand MCC"));
	LeftHand->SetupAttachment(RootComponent);
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UVirtualRealityUtilities::IsDesktopMode())
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
// and the master, request that the DCRA is attached to us.
void AVirtualRealityPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Only do this for all local controlled pawns
	if (IsLocallyControlled())
	{
		// Only do this for the primary node or when we're running in standalone
		if (UVirtualRealityUtilities::IsRoomMountedMode() && (UVirtualRealityUtilities::IsPrimaryNode() || GetNetMode())
			== NM_Standalone)
		{
			// If we are also the authority (standalone or listen server), directly attach it to us.
			// If we are not (client), ask the server to do it.
			if (HasAuthority())
				AttachDCRAtoPawn();
			else
				ServerAttachDCRAtoPawnRpc();
		}
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

	SetupMotionControllerSources();

	// Should not do this here but on connection or on possess I think.
	if (ARWTHVRPlayerState* State = GetPlayerState<ARWTHVRPlayerState>())
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

	// Set up mappings on input extension components, need to do this nicely

	for (UActorComponent* Comp : GetComponentsByInterface(UInputExtensionInterface::StaticClass()))
	{
		Cast<IInputExtensionInterface>(Comp)->SetupPlayerInput(PlayerInputComponent);
	}
}

void AVirtualRealityPawn::EvaluateLivelink() const
{
	if (UVirtualRealityUtilities::IsRoomMountedMode() && IsLocallyControlled())
	{
		if (bDisableLiveLink || HeadSubjectRepresentation.Subject.IsNone() || HeadSubjectRepresentation.Role == nullptr)
		{
			return;
		}


		// Get the LiveLink interface and evaluate the current existing frame data for the given Subject and Role.
		ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(
			ILiveLinkClient::ModularFeatureName);
		FLiveLinkSubjectFrameData SubjectData;
		const bool bHasValidData = LiveLinkClient.EvaluateFrame_AnyThread(
			HeadSubjectRepresentation.Subject, HeadSubjectRepresentation.Role, SubjectData);

		if (!bHasValidData)
			return;

		// Assume we are using a Transform Role to track the components! This is a slightly dangerous assumption, and could be further improved.
		const FLiveLinkTransformStaticData* StaticData = SubjectData.StaticData.Cast<FLiveLinkTransformStaticData>();
		const FLiveLinkTransformFrameData* FrameData = SubjectData.FrameData.Cast<FLiveLinkTransformFrameData>();

		if (StaticData && FrameData)
		{
			// Finally, apply the transform to this component according to the static data.
			ApplyLiveLinkTransform(FrameData->Transform, *StaticData);
		}
	}
}

void AVirtualRealityPawn::UpdateRightHandForDesktopInteraction() const
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

// Todo rewrite this in some other way or attach it differently, this is horrible
// Executed on the server only: Finds and attaches the CaveSetup Actor, which contains the DCRA to the Pawn.
// It is only executed on the server because attachments are synced to all clients, but not from client to server.
void AVirtualRealityPawn::AttachDCRAtoPawn()
{
	if (!CaveSetupActorClass || !CaveSetupActorClass->IsValidLowLevelFast())
	{
		UE_LOGFMT(Toolkit, Warning, "No CaveSetup Actor class set in pawn!");
		return;
	}

	// Find our CaveSetup actor
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), CaveSetupActorClass, FoundActors);

	if (!FoundActors.IsEmpty())
	{
		const auto CaveSetupActor = FoundActors[0];
		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		AttachmentRules.RotationRule = EAttachmentRule::KeepWorld;
		CaveSetupActor->AttachToActor(this, AttachmentRules);
	}
	else
	{
		UE_LOGFMT(Toolkit, Warning,
		          "No CaveSetup Actor found which can be attached to the Pawn! This won't work on the Cave.");
	}
}

void AVirtualRealityPawn::SetupMotionControllerSources()
{
	// Setup Motion Controllers

	FName MotionControllerSourceLeft = EName::None;
	FName MotionControllerSourceRight = EName::None;
	if (UVirtualRealityUtilities::IsHeadMountedMode())
	{
		MotionControllerSourceLeft = FName("Left");
		MotionControllerSourceRight = FName("Right");
	}
	if (UVirtualRealityUtilities::IsRoomMountedMode())
	{
		MotionControllerSourceLeft = LeftSubjectRepresentation.Subject;
		MotionControllerSourceRight = RightSubjectRepresentation.Subject;
	}
	LeftHand->SetTrackingMotionSource(MotionControllerSourceLeft);
	RightHand->SetTrackingMotionSource(MotionControllerSourceRight);
}

// Requests the server to perform the attachment, as only the server can sync this to all the other clients.
void AVirtualRealityPawn::ServerAttachDCRAtoPawnRpc_Implementation()
{
	// We're on the server here - attach the actor to the pawn.
	AttachDCRAtoPawn();
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

void AVirtualRealityPawn::ApplyLiveLinkTransform(const FTransform& Transform,
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
