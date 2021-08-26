#include "CAVEOverlay/CAVEOverlayController.h"
#include "CoreMinimal.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "IDisplayCluster.h"
#include "IXRTrackingSystem.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Game/IDisplayClusterGameManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/DisplayClusterSceneComponent.h"
#include "Engine/CollisionProfile.h"
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

UStaticMeshComponent* ACAVEOverlayController::CreateMeshComponent(const FName& Name, UStaticMesh* Mesh, USceneComponent* Parent)
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
	AutoReceiveInput = EAutoReceiveInput::Player0;

	ConstructorHelpers::FClassFinder<UDoorOverlayData> WidgetClassFinder(TEXT("Blueprint'/RWTHVRToolkit/CAVEOverlay/DoorOverlay'"));
	if (WidgetClassFinder.Succeeded())
	{
		OverlayClass = WidgetClassFinder.Class;
	}
	else
	{
		UE_LOG(LogCAVEOverlay, Error, TEXT("Could not find the DoorOverlay class. Have you renamed it?"));
	}

	//Creation of sub-components
	Root = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(Root);
	TapeRoot = CreateDefaultSubobject<USceneComponent>("TapeRoot");
	SignRoot = CreateDefaultSubobject<USceneComponent>("SignRoot");
	TapeRoot->SetupAttachment(Root);
	SignRoot->SetupAttachment(Root);

	//Loading of Materials and Meshes
	UVirtualRealityUtilities::LoadAsset("/RWTHVRToolkit/CAVEOverlay/Stripes", TapeMaterial);
	UVirtualRealityUtilities::LoadAsset("/RWTHVRToolkit/CAVEOverlay/StopMaterial", SignMaterial);
	UVirtualRealityUtilities::LoadAsset("/RWTHVRToolkit/CAVEOverlay/Plane", PlaneMesh);

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

void ACAVEOverlayController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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

void ACAVEOverlayController::CycleDoorType()
{
	DoorCurrentMode = static_cast<EDoorMode>((DoorCurrentMode + 1) % DOOR_NUM_MODES);

	IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
	if (Manager)
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
	if (Event.Category.Equals("CAVEOverlay") && Event.Type.Equals("DoorChange") && Event.Parameters.Contains("NewDoorState"))
	{
		SetDoorMode(static_cast<EDoorMode>(FCString::Atoi(*Event.Parameters["NewDoorState"])));
	}
}

void ACAVEOverlayController::SetDoorMode(EDoorMode NewMode)
{
	DoorCurrentMode = NewMode;
	switch (DoorCurrentMode)
	{
	case EDoorMode::DOOR_DEBUG:
	case EDoorMode::DOOR_PARTIALLY_OPEN:
		DoorCurrentOpeningWidthAbsolute = DoorOpeningWidthAbsolute;
		if (ScreenType == SCREEN_DOOR) Overlay->BlackBox->SetRenderScale(FVector2D(0, 1));
		if (ScreenType == SCREEN_DOOR_PARTIAL) Overlay->BlackBox->SetRenderScale(FVector2D(DoorOpeningWidthRelative, 1));
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

	UE_LOG(LogCAVEOverlay, Log, TEXT("Switched door state to '%s'. New opening width is %f."), *DoorModeNames[DoorCurrentMode], DoorCurrentOpeningWidthAbsolute);

	if (ScreenType == SCREEN_MASTER)
	{
		Overlay->CornerText->SetText(FText::FromString(DoorModeNames[DoorCurrentMode]));
	}
}

// Called when the game starts or when spawned
void ACAVEOverlayController::BeginPlay()
{
	Super::BeginPlay();

	bCAVEMode = UVirtualRealityUtilities::IsCave(); /* Store current situation */

	if (!bCAVEMode) return; // Not our business

	//Input config
	InputComponent->BindKey(EKeys::F10, EInputEvent::IE_Pressed, this, &ACAVEOverlayController::CycleDoorType);
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventJsonListener::CreateUObject(this, &ACAVEOverlayController::HandleClusterEvent);
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

	Overlay = CreateWidget<UDoorOverlayData>(GetWorld()->GetFirstPlayerController(), OverlayClass);
	Overlay->AddToViewport(0);
	SetDoorMode(DoorCurrentMode);
	Overlay->CornerText->SetText(FText::FromString("")); //Set Text to "" until someone presses the key for the first time

	if (!bAttachedToCAVEOrigin && CaveOrigin)
	{
		AttachToComponent(CaveOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bAttachedToCAVEOrigin = true;
	}
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

float ACAVEOverlayController::CalculateOpacityFromPosition(FVector Position) const
{
	return FMath::Max(
		FMath::Clamp((FMath::Abs(Position.X) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0f, 1.0f),
		FMath::Clamp((FMath::Abs(Position.Y) - (WallDistance - WallCloseDistance)) / WallFadeDistance, 0.0f, 1.0f)
	);
}

bool ACAVEOverlayController::PositionInDoorOpening(FVector Position) const
{
	return FMath::IsWithinInclusive(-Position.X, WallDistance + 10 - 20 - WallCloseDistance, WallDistance + 10) //Overlap both sides 10cm
		&& FMath::IsWithinInclusive(-Position.Y, WallDistance + 10 - DoorCurrentOpeningWidthAbsolute, WallDistance + 10); //Overlap one side 10cm
}

// Called every frame
void ACAVEOverlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCAVEMode) return; // Not our business

	if (!CaveOrigin)
	{
		CaveOrigin = UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_CAVE_ORIGIN);
	}

	if (!bAttachedToCAVEOrigin && CaveOrigin)
	{
		AttachToComponent(CaveOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bAttachedToCAVEOrigin = true;
	}

	//FPS Counter
	if (Overlay)
	{
		if (DoorCurrentMode == EDoorMode::DOOR_DEBUG && ContainsFString(ScreensFPS, IDisplayCluster::Get().GetClusterMgr()->GetNodeId()))
		{
			Overlay->FPS->SetText(FText::FromString(FString::Printf(TEXT("FPS: %.1f"), 1.0f / DeltaTime)));
		}
		else
		{
			Overlay->FPS->SetText(FText::FromString(""));
		}
	}

	if (!Head)
	{
	    Head = UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_SHUTTERGLASSES);
	}

	if (!Head || !CaveOrigin) return; //Display Cluster not fully initialized

	//Head/Tape Logic
    const FVector ShutterPosition = Head->GetComponentLocation() - CaveOrigin->GetComponentLocation();
    const bool bHeadIsCloseToWall = FMath::IsWithinInclusive(ShutterPosition.GetAbsMax(), WallDistance - WallCloseDistance, WallDistance);

	if (bHeadIsCloseToWall && !PositionInDoorOpening(ShutterPosition))
	{
		TapeRoot->SetVisibility(true, true);
		TapeRoot->SetRelativeLocation(ShutterPosition * FVector(0, 0, 1)); //Only apply Z

		TapeMaterialDynamic->SetScalarParameterValue("BarrierOpacity", CalculateOpacityFromPosition(ShutterPosition));

		if (FMath::IsWithin(FVector2D(ShutterPosition).GetAbsMax(), WallDistance - WallWarningDistance, WallDistance))
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

	// Flystick/Sign Logic
    if(!Flystick)
	{
	    Flystick = UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_FLYSTICK);
	}
	if (Flystick)
	{
        const FVector FlystickPosition = Flystick->GetRelativeTransform().GetLocation();
        const bool bFlystickInDoor = PositionInDoorOpening(FlystickPosition);

		SignNegativeX->SetRelativeLocation(FVector(-WallDistance, FlystickPosition.Y, FlystickPosition.Z));
		SignNegativeY->SetRelativeLocation(FVector(FlystickPosition.X, -WallDistance, FlystickPosition.Z));
		SignPositiveX->SetRelativeLocation(FVector(+WallDistance, FlystickPosition.Y, FlystickPosition.Z));
		SignPositiveY->SetRelativeLocation(FVector(FlystickPosition.X, +WallDistance, FlystickPosition.Z));

		SignNegativeX->SetVisibility(FMath::IsWithin(-FlystickPosition.X, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
		SignNegativeY->SetVisibility(FMath::IsWithin(-FlystickPosition.Y, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
		SignPositiveX->SetVisibility(FMath::IsWithin(+FlystickPosition.X, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);
		SignPositiveY->SetVisibility(FMath::IsWithin(+FlystickPosition.Y, WallDistance - WallCloseDistance, WallDistance) && !bFlystickInDoor);

		SignMaterialDynamic->SetScalarParameterValue("SignOpacity", CalculateOpacityFromPosition(FlystickPosition));
	}
	else
	{
		SignNegativeX->SetVisibility(false);
		SignNegativeY->SetVisibility(false);
		SignPositiveX->SetVisibility(false);
		SignPositiveY->SetVisibility(false);
	}
}
