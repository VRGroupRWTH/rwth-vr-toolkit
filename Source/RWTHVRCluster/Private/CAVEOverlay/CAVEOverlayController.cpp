#include "CAVEOverlay/CAVEOverlayController.h"

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "IDisplayCluster.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Logging/StructuredLog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Pawn/RWTHVRPawn.h"
#include "Utility/RWTHVRUtilities.h"


DEFINE_LOG_CATEGORY(LogCAVEOverlay);

// Helper function to check if a string is contained within an array of strings, ignoring case.
bool ContainsFString(const TArray<FString>& Array, const FString& Entry)
{
	for (FString Current_Entry : Array)
	{
		if (Current_Entry.Equals(Entry, ESearchCase::IgnoreCase))
			return true;
	}
	return false;
}

UStaticMeshComponent* ACAVEOverlayController::CreateMeshComponent(const FName& Name, USceneComponent* Parent)
{
	UStaticMeshComponent* Result = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Result->SetupAttachment(Parent);
	Result->SetVisibility(false);
	Result->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	return Result;
}

// Sets default values
ACAVEOverlayController::ACAVEOverlayController()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;

	// Creation of sub-components
	Root = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(Root);

	Tape = CreateMeshComponent("Tape", Root);
	SignRightHand = CreateMeshComponent("SignRightHand", Root);
	SignLeftHand = CreateMeshComponent("SignLeftHand", Root);
}

void ACAVEOverlayController::CycleDoorType()
{
	DoorCurrentMode = static_cast<EDoorMode>((DoorCurrentMode + 1) % DOOR_NUM_MODES);

	// Send out a cluster event to the whole cluster that the door mode has been changed
	if (auto* const Manager = IDisplayCluster::Get().GetClusterMgr())
	{
		FDisplayClusterClusterEventJson cluster_event;
		cluster_event.Name = "CAVEOverlay Change Door to " + DoorModeNames[DoorCurrentMode];
		cluster_event.Type = "DoorChange";
		cluster_event.Category = "CAVEOverlay";
		cluster_event.Parameters.Add("NewDoorState", FString::FromInt(DoorCurrentMode));
		Manager->EmitClusterEventJson(cluster_event, true);
	}
}

void ACAVEOverlayController::HandleClusterEvent(const FDisplayClusterClusterEventJson& Event)
{
	if (Event.Category.Equals("CAVEOverlay") && Event.Type.Equals("DoorChange") &&
		Event.Parameters.Contains("NewDoorState"))
	{
		SetDoorMode(static_cast<EDoorMode>(FCString::Atoi(*Event.Parameters["NewDoorState"])));
	}
}

void ACAVEOverlayController::SetDoorMode(const EDoorMode NewMode)
{
	DoorCurrentMode = NewMode;
	switch (DoorCurrentMode)
	{
	case EDoorMode::DOOR_DEBUG:
	case EDoorMode::DOOR_PARTIALLY_OPEN:
		DoorCurrentOpeningWidthAbsolute = DoorOpeningWidthAbsolute;
		if (ScreenType == SCREEN_DOOR)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL)
			Overlay->BlackBox->SetRenderScale(FVector2D(DoorOpeningWidthRelative, 1));
		if (ScreenType == SCREEN_PRIMARY)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));

		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoorMode::DOOR_OPEN:
		DoorCurrentOpeningWidthAbsolute = WallDistance * 2;
		if (ScreenType == SCREEN_DOOR)
			Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL)
			Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (ScreenType == SCREEN_PRIMARY)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));

		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoorMode::DOOR_CLOSED:
		DoorCurrentOpeningWidthAbsolute = 0;
		if (ScreenType == SCREEN_DOOR)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_PRIMARY)
			Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));

		Overlay->BlackBox->SetVisibility(ESlateVisibility::Hidden);
		break;
	default:;
	}

	// On the secondary nodes that are not the door, hide the overlay completely
	// It might make more sense to just not add it there...
	if (ScreenType == SCREEN_NORMAL)
		Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));

	UE_LOGFMT(LogCAVEOverlay, Log, "Switched door state to {State}. New opening width is {Width}.",
			  *DoorModeNames[DoorCurrentMode], DoorCurrentOpeningWidthAbsolute);

	// On the primary node, show which door mode is currently active.
	if (ScreenType == SCREEN_PRIMARY)
	{
		Overlay->CornerText->SetText(FText::FromString(DoorModeNames[DoorCurrentMode]));
	}
}

// Called when the game starts or when spawned
void ACAVEOverlayController::BeginPlay()
{
	Super::BeginPlay();

	// Don't do anything if we're a dedicated server. We shouldn't even exist there.
	if (GetNetMode() == NM_DedicatedServer)
		return;

	// Currently, there is no support for multi-user systems in general, as we only depend on the local pawn.
	// In a MU setting, the relevant pawn isn't our local one, but the primary node's pawn.
	if (GetNetMode() != NM_Standalone)
		return;

	// This should return the respective client's local playercontroller or, if we're a listen server, our own PC.
	auto* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

	// it can happen that the PC is valid, but we have no player attached to it yet.
	// Check for this - however, we should work around it by somehow getting notified when that happens.
	// Not sure which place would be best...
	const bool bValidPC = PC && PC->GetLocalPlayer();

	if (!bValidPC || !URWTHVRUtilities::IsRoomMountedMode())
		return;

	// Input config
	if (URWTHVRUtilities::IsPrimaryNode())
	{
		if (CycleDoorTypeInputAction == nullptr)
		{
			UE_LOGFMT(LogCAVEOverlay, Error, "Input action and mapping not set in CaveOverlayController!");
			return;
		}

		UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PC->InputComponent);
		Input->BindAction(CycleDoorTypeInputAction, ETriggerEvent::Triggered, this,
						  &ACAVEOverlayController::CycleDoorType);
	}

	// Bind the cluster events that manage the door state.
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate =
			FOnClusterEventJsonListener::CreateUObject(this, &ACAVEOverlayController::HandleClusterEvent);
		ClusterManager->AddClusterEventJsonListener(ClusterEventListenerDelegate);
	}

	// Determine the screen-type for later usage
	if (IDisplayCluster::Get().GetClusterMgr()->GetNodeId().Equals(ScreenMain, ESearchCase::IgnoreCase))
	{
		ScreenType = SCREEN_PRIMARY;
	}
	else if (ContainsFString(ScreensDoor, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
	{
		ScreenType = SCREEN_DOOR;
	}
	else if (ContainsFString(ScreensDoorPartial, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
	{
		ScreenType = SCREEN_DOOR_PARTIAL;
	}
	else
	{
		ScreenType = SCREEN_NORMAL;
	}

	// Create and add widget to local playercontroller.
	if (!OverlayClass)
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "OverlayClass not set in CaveOverlayController!");
		return;
	}

	Overlay = CreateWidget<UDoorOverlayData>(PC, OverlayClass);
	Overlay->AddToViewport(0);

	// Set the default door mode (partially open)
	SetDoorMode(DoorCurrentMode);

	// Set Text to "" until someone presses the key for the first time
	Overlay->CornerText->SetText(FText::FromString(""));

	// Get the pawn so we can have access to head and hand positions
	VRPawn = Cast<ARWTHVRPawn>(PC->GetPawnOrSpectator());
	if (VRPawn)
	{
		// we're good to go!
		bInitialized = true;
	}
	else
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "No VirtualRealityPawn found which we could attach to!");
	}

	// Create dynamic materials at runtime
	TapeMaterialDynamic = Tape->CreateDynamicMaterialInstance(0);
	RightSignMaterialDynamic = SignRightHand->CreateDynamicMaterialInstance(0);
	LeftSignMaterialDynamic = SignLeftHand->CreateDynamicMaterialInstance(0);

	UE_LOGFMT(LogCAVEOverlay, Display, "CaveOverlay Initialization was successfull.");
}

void ACAVEOverlayController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && ClusterEventListenerDelegate.IsBound())
	{
		ClusterManager->RemoveClusterEventJsonListener(ClusterEventListenerDelegate);
	}

	Super::EndPlay(EndPlayReason);
}

double ACAVEOverlayController::CalculateOpacityFromPosition(const FVector& Position) const
{
	// Calculate opacity value depending on how far we are from the walls. Further away == lower opacity,
	// fully opaque when WallFadeDistance away from the wall. Could just use a lerp here..
	return FMath::Max(
		FMath::Clamp((FMath::Abs(Position.X) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0, 1.0),
		FMath::Clamp((FMath::Abs(Position.Y) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0, 1.0));
}

bool ACAVEOverlayController::PositionInDoorOpening(const FVector& Position) const
{
	// The position of the corner with 10cm of buffer. In negative direction because the door is in negative direction
	// of the cave
	const float CornerValue = -(WallDistance + 10);

	// Check whether our X position is within the door zone. This zone starts 10cm further away from the wall
	// than the WallCloseDistance, and ends 10cm outside of the wall (door). As the door is in negative X direction,
	// the signs need to be negated.
	const bool bXWithinDoor =
		FMath::IsWithinInclusive(Position.X, CornerValue, -(WallDistance - WallCloseDistance - 10));

	// Checks whether our Y position is between the lower corner with some overlap and
	// the door corner (CornerValue + DoorCurrentOpeningWidthAbsolute)
	const bool bYWithinDoor =
		FMath::IsWithinInclusive(Position.Y, CornerValue, CornerValue + DoorCurrentOpeningWidthAbsolute);
	return bXWithinDoor && bYWithinDoor;
}

void ACAVEOverlayController::SetSignsForHand(UStaticMeshComponent* Sign, const FVector& HandPosition,
											 UMaterialInstanceDynamic* HandMaterial) const
{
	const bool bHandIsCloseToWall =
		FMath::IsWithinInclusive(HandPosition.GetAbsMax(), WallDistance - WallCloseDistance, WallDistance);
	if (bHandIsCloseToWall && !PositionInDoorOpening(HandPosition) && Sign && HandMaterial)
	{
		Sign->SetVisibility(true);
		HandMaterial->SetScalarParameterValue("SignOpacity", CalculateOpacityFromPosition(HandPosition));

		// Which wall are we closest to? This is the wall we project the sign onto
		const bool bXWallCloser = FMath::Abs(HandPosition.X) > FMath::Abs(HandPosition.Y);

		// Set the position towards the closest wall to the wall itself, keep the other positions
		const double X = bXWallCloser ? FMath::Sign(HandPosition.X) * WallDistance : HandPosition.X;
		const double Y = bXWallCloser ? HandPosition.Y : FMath::Sign(HandPosition.Y) * WallDistance;
		const double Z = HandPosition.Z;

		// Rotate the sign by 90° if we're on a side wall
		const auto Rot = bXWallCloser ? FRotator(0, 0, 0) : FRotator(0, 90, 0);
		const auto Pos = FVector(X, Y, Z);
		Sign->SetRelativeLocationAndRotation(Pos, Rot);
	}
	else if (Sign)
	{
		Sign->SetVisibility(false);
	}
}

void ACAVEOverlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If we're not yet initialized, do nothing. This shouldn't really happen as we only spawn on the cave anyway
	if (!bInitialized)
	{
		return;
	}

	// Head/Tape Logic
	const FVector HeadPosition = VRPawn->HeadCameraComponent->GetRelativeTransform().GetLocation();
	const bool bHeadIsCloseToWall =
		FMath::IsWithinInclusive(HeadPosition.GetAbsMax(), WallDistance - WallCloseDistance, WallDistance);

	// Only show the tape when close to a wall and not within the door opening
	if (bHeadIsCloseToWall && !PositionInDoorOpening(HeadPosition))
	{
		Tape->SetVisibility(true);

		// Offset the tape in z direction to always be at head height
		Tape->SetRelativeLocation(HeadPosition * FVector(0, 0, 1));

		TapeMaterialDynamic->SetScalarParameterValue("BarrierOpacity", CalculateOpacityFromPosition(HeadPosition));

		if (FMath::IsWithin(FVector2D(HeadPosition).GetAbsMax(), WallDistance - WallWarningDistance, WallDistance))
		{
			// in warning distance == red tape
			TapeMaterialDynamic->SetVectorParameterValue("StripeColor", FVector(1, 0, 0));
		}
		else
		{
			// otherwise we're just yellow
			TapeMaterialDynamic->SetVectorParameterValue("StripeColor", FVector(1, 1, 0));
		}
	}
	else
	{
		Tape->SetVisibility(false);
	}

	// Hand Logic
	const FVector RightPosition = VRPawn->RightHand->GetRelativeTransform().GetLocation();
	const FVector LeftPosition = VRPawn->LeftHand->GetRelativeTransform().GetLocation();

	// Set the position rotation, opacity, visibility of the hand warning signs.
	SetSignsForHand(SignRightHand, RightPosition, RightSignMaterialDynamic);
	SetSignsForHand(SignLeftHand, LeftPosition, LeftSignMaterialDynamic);
}
