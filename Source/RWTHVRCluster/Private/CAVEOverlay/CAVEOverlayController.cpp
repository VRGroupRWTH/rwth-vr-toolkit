#include "CAVEOverlay/CAVEOverlayController.h"

#include "CoreMinimal.h"
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
#include "Pawn/VirtualRealityPawn.h"
#include "Utility/VirtualRealityUtilities.h"


DEFINE_LOG_CATEGORY(LogCAVEOverlay);

bool ContainsFString(const TArray<FString>& Array, const FString& Entry)
{
	for (FString Current_Entry : Array)
	{
		if (Current_Entry.Equals(Entry, ESearchCase::IgnoreCase)) return true;
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
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;

	//Creation of sub-components
	Root = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(Root);

	Tape = CreateMeshComponent("Tape", Root);
	SignRightHand = CreateMeshComponent("SignRightHand", Root);
	SignLeftHand = CreateMeshComponent("SignLeftHand", Root);
}

void ACAVEOverlayController::CycleDoorType()
{
	DoorCurrentMode = static_cast<EDoorMode>((DoorCurrentMode + 1) % DOOR_NUM_MODES);

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
	if (Event.Category.Equals("CAVEOverlay") && Event.Type.Equals("DoorChange") && Event.Parameters.Contains(
		"NewDoorState"))
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
			Overlay->BlackBox->
			         SetRenderScale(FVector2D(DoorOpeningWidthRelative, 1));
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
	default: ;
	}

	if (ScreenType == SCREEN_NORMAL)
		Overlay->BlackBox->SetRenderScale(FVector2D(0, 1)); //no overlay

	UE_LOGFMT(LogCAVEOverlay, Log, "Switched door state to {State}. New opening width is {Width}.",
	          *DoorModeNames[DoorCurrentMode], DoorCurrentOpeningWidthAbsolute);

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

	// This should return the respective client's local playercontroller or, if we're a listen server, our own PC.
	auto* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

	const bool bValidPC = PC != nullptr;

	if (!bValidPC || !UVirtualRealityUtilities::IsRoomMountedMode())
		return;

	//Input config
	//InputComponent->BindKey(EKeys::F10, EInputEvent::IE_Pressed, this, &ACAVEOverlayController::CycleDoorType);
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventJsonListener::CreateUObject(
			this, &ACAVEOverlayController::HandleClusterEvent);
		ClusterManager->AddClusterEventJsonListener(ClusterEventListenerDelegate);
	}

	//Determine the screen-type for later usage
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
	SetDoorMode(DoorCurrentMode);
	//Set Text to "" until someone presses the key for the first time
	Overlay->CornerText->SetText(FText::FromString(""));

	VRPawn = Cast<AVirtualRealityPawn>(PC->GetPawnOrSpectator());
	if (VRPawn)
	{
		//AttachToActor(VRPawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bInitialized = true;
	}
	else
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "No VirtualRealityPawn found which we could attach to!");
	}

	//Create dynamic materials in runtime
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
	return FMath::Max(
		FMath::Clamp((FMath::Abs(Position.X) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0, 1.0),
		FMath::Clamp((FMath::Abs(Position.Y) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0, 1.0)
	);
}

bool ACAVEOverlayController::PositionInDoorOpening(const FVector& Position) const
{
	//Overlap both sides 10cm
	return FMath::IsWithinInclusive(-Position.X, WallDistance + 10 - 20 - WallCloseDistance, WallDistance + 10)
		//Overlap one side 10cm
		&& FMath::IsWithinInclusive(-Position.Y, WallDistance + 10 - DoorCurrentOpeningWidthAbsolute,
		                            WallDistance + 10);
}

void ACAVEOverlayController::SetSignsForHand(UStaticMeshComponent* Sign, const FVector& HandPosition,
                                             UMaterialInstanceDynamic* HandMaterial) const
{
	const bool bHandIsCloseToWall = FMath::IsWithinInclusive(HandPosition.GetAbsMax(),
	                                                         WallDistance - WallCloseDistance, WallDistance);
	if (bHandIsCloseToWall && !PositionInDoorOpening(HandPosition))
	{
		Sign->SetVisibility(true);
		HandMaterial->SetScalarParameterValue("SignOpacity",
		                                      CalculateOpacityFromPosition(HandPosition));

		// Which wall are we closest to? This is the wall we project the sign onto
		const bool bXWallCloser = FMath::Abs(HandPosition.X) > FMath::Abs(HandPosition.Y);

		// Set the position towards the closest wall to the wall itself, keep the other positions
		const double X = bXWallCloser ? FMath::Sign(HandPosition.X) * WallDistance : HandPosition.X;
		const double Y = bXWallCloser ? HandPosition.Y : FMath::Sign(HandPosition.Y) * WallDistance;
		const double Z = HandPosition.Z;

		const auto Rot = bXWallCloser ? FRotator(0, 0, 0) : FRotator(0, 90, 0);
		const auto Pos = FVector(X, Y, Z);
		Sign->SetRelativeLocationAndRotation(Pos, Rot);

		UE_LOGFMT(LogCAVEOverlay, Log, "HandPos: {Hand} vs SignPos: {Sign}", HandPosition.ToString(), Pos.ToString());
	}
	else
	{
		Sign->SetVisibility(false);
	}
}

void ACAVEOverlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInitialized)
	{
		return;
	}

	//Head/Tape Logic
	const FVector HeadPosition = VRPawn->HeadCameraComponent->GetRelativeTransform().GetLocation();
	const bool bHeadIsCloseToWall = FMath::IsWithinInclusive(HeadPosition.GetAbsMax(),
	                                                         WallDistance - WallCloseDistance, WallDistance);
	if (bHeadIsCloseToWall && !PositionInDoorOpening(HeadPosition))
	{
		Tape->SetVisibility(true);
		Tape->SetRelativeLocation(HeadPosition * FVector(0, 0, 1)); //Only apply Z

		TapeMaterialDynamic->SetScalarParameterValue("BarrierOpacity", CalculateOpacityFromPosition(HeadPosition));

		if (FMath::IsWithin(FVector2D(HeadPosition).GetAbsMax(), WallDistance - WallWarningDistance, WallDistance))
		{
			//in warning distance == red tape
			TapeMaterialDynamic->SetVectorParameterValue("StripeColor", FVector(1, 0, 0));
		}
		else
		{
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

	SetSignsForHand(SignRightHand, RightPosition, RightSignMaterialDynamic);
	SetSignsForHand(SignLeftHand, LeftPosition, LeftSignMaterialDynamic);
}
