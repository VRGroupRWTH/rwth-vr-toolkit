#include "CAVEOverlay/CAVEOverlayController.h"

#include "CoreMinimal.h"
#include "IDisplayCluster.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "CAVEOverlay/DoorOverlayData.h"
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

UStaticMeshComponent* ACAVEOverlayController::CreateMeshComponent(const FName& Name, UStaticMesh* Mesh,
                                                                  USceneComponent* Parent)
{
	UStaticMeshComponent* Result = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Result->SetStaticMesh(Mesh);
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
	TapeRoot = CreateDefaultSubobject<USceneComponent>("TapeRoot");
	SignRoot = CreateDefaultSubobject<USceneComponent>("SignRoot");
	TapeRoot->SetupAttachment(Root);
	SignRoot->SetupAttachment(Root);

	TapeNegativeY = CreateMeshComponent("TapeNegY", PlaneMesh, TapeRoot);
	TapeNegativeX = CreateMeshComponent("TapeNegX", PlaneMesh, TapeRoot);
	TapePositiveY = CreateMeshComponent("TapePosY", PlaneMesh, TapeRoot);
	TapePositiveX = CreateMeshComponent("TapePosX", PlaneMesh, TapeRoot);

	SignNegativeY = CreateMeshComponent("SignNegY", PlaneMesh, SignRoot);
	SignNegativeX = CreateMeshComponent("SignNegX", PlaneMesh, SignRoot);
	SignPositiveY = CreateMeshComponent("SignPosY", PlaneMesh, SignRoot);
	SignPositiveX = CreateMeshComponent("SignPosX", PlaneMesh, SignRoot);

	//Set initial Position, Rotation and Scale of Tape
	TapeNegativeY->SetRelativeLocationAndRotation(FVector(0, -WallDistance, 0), FRotator(0, 0, 90));
	TapePositiveY->SetRelativeLocationAndRotation(FVector(0, +WallDistance, 0), FRotator(0, 180, 90));
	TapeNegativeX->SetRelativeLocationAndRotation(FVector(-WallDistance, 0, 0), FRotator(0, -90, 90));
	TapePositiveX->SetRelativeLocationAndRotation(FVector(+WallDistance, 0, 0), FRotator(0, 90, 90));

	TapeNegativeY->SetRelativeScale3D(FVector(WallDistance / 100 * 2, 0.15, 1));
	TapePositiveY->SetRelativeScale3D(FVector(WallDistance / 100 * 2, 0.15, 1));
	TapeNegativeX->SetRelativeScale3D(FVector(WallDistance / 100 * 2, 0.15, 1));
	TapePositiveX->SetRelativeScale3D(FVector(WallDistance / 100 * 2, 0.15, 1));

	//Set initial Position, Rotation and Scale of Signs
	SignNegativeY->SetRelativeLocationAndRotation(FVector(0, -WallDistance, 0), FRotator(0, 0, 90));
	SignPositiveY->SetRelativeLocationAndRotation(FVector(0, +WallDistance, 0), FRotator(0, 180, 90));
	SignNegativeX->SetRelativeLocationAndRotation(FVector(-WallDistance, 0, 0), FRotator(0, -90, 90));
	SignPositiveX->SetRelativeLocationAndRotation(FVector(+WallDistance, 0, 0), FRotator(0, 90, 90));

	SignNegativeY->SetRelativeScale3D(FVector(0.5f));
	SignPositiveY->SetRelativeScale3D(FVector(0.5f));
	SignNegativeX->SetRelativeScale3D(FVector(0.5f));
	SignPositiveX->SetRelativeScale3D(FVector(0.5f));
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
		if (ScreenType == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL)
			Overlay->BlackBox->
			         SetRenderScale(FVector2D(DoorOpeningWidthRelative, 1));
		if (ScreenType == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoorMode::DOOR_OPEN:
		DoorCurrentOpeningWidthAbsolute = WallDistance * 2;
		if (ScreenType == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(1, 1));
		if (ScreenType == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case EDoorMode::DOOR_CLOSED:
		DoorCurrentOpeningWidthAbsolute = 0;
		if (ScreenType == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_MASTER) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		Overlay->BlackBox->SetVisibility(ESlateVisibility::Hidden);
		break;
	default: ;
	}
	if (ScreenType == SCREEN_NORMAL) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1)); //no overlay

	UE_LOG(LogCAVEOverlay, Log, TEXT("Switched door state to '%s'. New opening width is %f."),
	       *DoorModeNames[DoorCurrentMode], DoorCurrentOpeningWidthAbsolute);

	if (ScreenType == SCREEN_MASTER)
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

	bCAVEMode = UVirtualRealityUtilities::IsCave(); /* Store current situation */

	bCAVEMode = true;

	if (PC == nullptr || !bCAVEMode)
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
		ScreenType = SCREEN_MASTER;
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

	// Attach it to the pawn
	VRPawn = Cast<AVirtualRealityPawn>(PC->GetPawnOrSpectator());
	if (VRPawn)
	{
		AttachToActor(VRPawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bAttachedToPawn = true;
	}
	else
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "No VirtualRealityPawn found which we could attach to!");
	}

	if (!TapeMaterial || !TapeMaterial->IsValidLowLevelFast())
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "TapeMaterial not set! Please set asset reference in BP!");
		return;
	}

	if (!SignMaterial || !SignMaterial->IsValidLowLevelFast())
	{
		UE_LOGFMT(LogCAVEOverlay, Error, "SignMaterial not set! Please set asset reference in BP!");
		return;
	}

	//Create dynamic materials in runtime
	TapeMaterialDynamic = UMaterialInstanceDynamic::Create(TapeMaterial, TapeRoot);
	SignMaterialDynamic = UMaterialInstanceDynamic::Create(SignMaterial, SignRoot);

	TapeNegativeY->SetMaterial(0, TapeMaterialDynamic);
	TapeNegativeX->SetMaterial(0, TapeMaterialDynamic);
	TapePositiveY->SetMaterial(0, TapeMaterialDynamic);
	TapePositiveX->SetMaterial(0, TapeMaterialDynamic);

	SignNegativeY->SetMaterial(0, SignMaterialDynamic);
	SignNegativeX->SetMaterial(0, SignMaterialDynamic);
	SignPositiveY->SetMaterial(0, SignMaterialDynamic);
	SignPositiveX->SetMaterial(0, SignMaterialDynamic);
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

float ACAVEOverlayController::CalculateOpacityFromPosition(const FVector& Position) const
{
	return FMath::Max(
		FMath::Clamp((FMath::Abs(Position.X) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0f, 1.0f),
		FMath::Clamp((FMath::Abs(Position.Y) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0f, 1.0f)
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

void ACAVEOverlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// todo: avoid empty ticks, we should disable ourselves and/or never spawn when not in Cave mode
	if (!bCAVEMode)
		return;

	// todo: handle pawn changes etc
	if (!bAttachedToPawn)
	{
		return;
	}

	//Head/Tape Logic
	const FVector HeadPosition = VRPawn->HeadCameraComponent->GetRelativeTransform().GetLocation();
	const bool bHeadIsCloseToWall = FMath::IsWithinInclusive(HeadPosition.GetAbsMax(),
	                                                         WallDistance - WallCloseDistance, WallDistance);

	if (bHeadIsCloseToWall && !PositionInDoorOpening(HeadPosition))
	{
		TapeRoot->SetVisibility(true, true);
		TapeRoot->SetRelativeLocation(HeadPosition * FVector(0, 0, 1)); //Only apply Z

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
		TapeRoot->SetVisibility(false, true);
	}

	// Right Hand (Flystick) / Sign Logic

	const FVector RightHandPosition = VRPawn->RightHand->GetRelativeTransform().GetLocation();
	const bool bFlystickInDoor = PositionInDoorOpening(RightHandPosition);

	SignNegativeX->SetRelativeLocation(FVector(-WallDistance, RightHandPosition.Y, RightHandPosition.Z));
	SignNegativeY->SetRelativeLocation(FVector(RightHandPosition.X, -WallDistance, RightHandPosition.Z));
	SignPositiveX->SetRelativeLocation(FVector(+WallDistance, RightHandPosition.Y, RightHandPosition.Z));
	SignPositiveY->SetRelativeLocation(FVector(RightHandPosition.X, +WallDistance, RightHandPosition.Z));

	SignNegativeX->SetVisibility(
		FMath::IsWithin(-RightHandPosition.X, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
	SignNegativeY->SetVisibility(
		FMath::IsWithin(-RightHandPosition.Y, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
	SignPositiveX->SetVisibility(
		FMath::IsWithin(+RightHandPosition.X, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
	SignPositiveY->SetVisibility(
		FMath::IsWithin(+RightHandPosition.Y, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);

	SignMaterialDynamic->SetScalarParameterValue("SignOpacity", CalculateOpacityFromPosition(RightHandPosition));
}
