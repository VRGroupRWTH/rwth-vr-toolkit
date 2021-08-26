#include "CalibratioActor.h"

#include "Components/StaticMeshComponent.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "IDisplayCluster.h"
#include "TimerManager.h"
#include "CalibratioOverlay.h"
#include "GameFramework/InputSettings.h"
#include "Utility/VirtualRealityUtilities.h"

ACalibratioActor::ACalibratioActor()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	/* Loads needed Assets */
	UStaticMesh* CylinderMesh = nullptr;
	UVirtualRealityUtilities::LoadClass("WidgetBlueprint'/RWTHVRToolkit/Calibratio/CalibratioHud'", Overlay_Class);
	UVirtualRealityUtilities::LoadAsset("StaticMesh'/RWTHVRToolkit/Calibratio/Cylinder'", CylinderMesh);
	UVirtualRealityUtilities::LoadAsset("Material'/RWTHVRToolkit/Calibratio/CalibratioMaterial'", BaseMaterial);

	/* Create Mesh component and initialize */
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetStaticMesh(CylinderMesh);
	Mesh->SetRelativeScale3D(FVector(1,1,0.1f)); //Make it a Disk
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void ACalibratioActor::BeginPlay()
{
	Super::BeginPlay();

	/* Register cluster event listener */
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventJsonListener::CreateUObject(this, &ACalibratioActor::HandleClusterEvent);
		ClusterManager->AddClusterEventJsonListener(ClusterEventListenerDelegate);
	}

	/* Create Overlay */
	Overlay = CreateWidget<UCalibratioOverlay>(GetWorld()->GetFirstPlayerController(), Overlay_Class);
	Overlay->AddToViewport(0);
	Overlay->SetThresholds(MinRotation, MaxRotation, Threshold);
	Overlay->SetOwner(this);

	/* Bind Buttons */
	Overlay->ResettingButton->OnClicked.AddDynamic(this, &ACalibratioActor::ClusterReset);
	Overlay->IncreaseThresholdButton->OnClicked.AddDynamic(this, &ACalibratioActor::ClusterIncreaseThreshold);
	Overlay->DecreaseThresholdButton->OnClicked.AddDynamic(this, &ACalibratioActor::ClusterDecreaseThreshold);

	/* Hide this overlay on all clients */
	if (UVirtualRealityUtilities::IsSlave())
	{
		Overlay->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ACalibratioActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/* Create dynamic materials at runtime (Not constructor) */
	DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, RootComponent);
	DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);
	Mesh->SetMaterial(0, DynamicMaterial);
}

void ACalibratioActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/* Remove from Cluster events */
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && ClusterEventListenerDelegate.IsBound())
	{
		ClusterManager->RemoveClusterEventJsonListener(ClusterEventListenerDelegate);
	}

	Super::EndPlay(EndPlayReason);

	/* Remove components */
	GetWorld()->GetTimerManager().ClearTimer(ResetTimerHandle);
	if(Overlay->IsInViewport()) Overlay->RemoveFromViewport();
}

void ACalibratioActor::HandleClusterEvent(const FDisplayClusterClusterEventJson& Event)
{
	if(!Event.Category.Equals("Calibratio")) return; //Not our Business
	
	if (Event.Name == "CalibratioReset")
	{
		LocalReset();
	}
	else if (Event.Name == "CalibratioSetThreshold" && Event.Parameters.Contains("NewThreshold"))
	{
		LocalSetThreshold(FCString::Atof(*Event.Parameters["NewThreshold"]));
	}
	else if (Event.Name == "CalibratioArmAndSetCalibration" && Event.Parameters.Contains("NewMin") && Event.Parameters.Contains("NewMax"))
	{
		LocalArmAndSetCalibration(FCString::Atof(*Event.Parameters["NewMin"]), FCString::Atof(*Event.Parameters["NewMax"]));
	}
	else if (Event.Name == "CalibratioDespawn")
	{
		LocalDespawn();
	}
}

void ACalibratioActor::ClusterReset()
{
	if(UVirtualRealityUtilities::IsRoomMountedMode())
	{
		IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
		if (!Manager) return;

		FDisplayClusterClusterEventJson ClusterEvent;
		ClusterEvent.Name = "CalibratioReset";
		ClusterEvent.Category = "Calibratio";
		Manager->EmitClusterEventJson(ClusterEvent, true);
	}
	else if(UVirtualRealityUtilities::IsDesktopMode())
	{
		LocalReset();
	}
}

void ACalibratioActor::LocalReset()
{
	DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);
	CurrentCalibrationRuns = 0;
	MaxRotation = -FLT_MAX;
	MinRotation = FLT_MAX;
	LastRotations.Empty();
	LastRotations.Reserve(MaxCalibrationRuns);
}

void ACalibratioActor::ClusterIncreaseThreshold()
{
	ClusterChangeThreshold(FMath::DegreesToRadians(0.1f));
}

void ACalibratioActor::ClusterDecreaseThreshold()
{
	ClusterChangeThreshold(-FMath::DegreesToRadians(0.1f));
}

void ACalibratioActor::ClusterChangeThreshold(float Value)
{
	const float NewThreshold = Threshold + Value;

	if(UVirtualRealityUtilities::IsRoomMountedMode())
	{
		IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
		if (!Manager) return;

		FDisplayClusterClusterEventJson ClusterEvent;
		ClusterEvent.Name = "CalibratioSetThreshold";
		ClusterEvent.Category = "Calibratio";
		ClusterEvent.Parameters.Add("NewThreshold",FString::SanitizeFloat(NewThreshold));
		Manager->EmitClusterEventJson(ClusterEvent, true);
	}
	else if(UVirtualRealityUtilities::IsDesktopMode())
	{
		LocalSetThreshold(NewThreshold);		
	}
}

void ACalibratioActor::LocalSetThreshold(float NewValue)
{
	Threshold = NewValue;
	Overlay->SetThresholds(MinRotation, MaxRotation, Threshold);
}

void ACalibratioActor::ClusterArmAndSetCalibration(float MinAngle, float MaxAngle)
{
	if(UVirtualRealityUtilities::IsRoomMountedMode())
	{
		IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
		if (!Manager) return;

		FDisplayClusterClusterEventJson ClusterEvent;
		ClusterEvent.Name = "CalibratioArmAndSetCalibration";
		ClusterEvent.Category = "Calibratio";
		ClusterEvent.Parameters.Add("NewMin",FString::SanitizeFloat(MinAngle));
		ClusterEvent.Parameters.Add("NewMax",FString::SanitizeFloat(MaxAngle));
		Manager->EmitClusterEventJson(ClusterEvent, true);
	}
	else if(UVirtualRealityUtilities::IsDesktopMode())
	{
		LocalArmAndSetCalibration(MinAngle, MaxAngle);
	}
}

void ACalibratioActor::LocalArmAndSetCalibration(float NewMin, float NewMax)
{
	MinRotation = NewMin;
	MaxRotation = NewMax;
	Overlay->SetStatus(Waiting);
	DynamicMaterial->SetVectorParameterValue("Color", FColor::Black);
	CurrentCalibrationRuns = MaxCalibrationRuns + 1; //Arm
}

void ACalibratioActor::ClusterDespawn()
{
	if (UVirtualRealityUtilities::IsRoomMountedMode())
	{
		IDisplayClusterClusterManager* const Manager = IDisplayCluster::Get().GetClusterMgr();
		if (!Manager) return;

		FDisplayClusterClusterEventJson ClusterEvent;
		ClusterEvent.Name = "CalibratioDespawn";
		ClusterEvent.Category = "Calibratio";
		Manager->EmitClusterEventJson(ClusterEvent, true);
	}
	else if (UVirtualRealityUtilities::IsDesktopMode())
	{
		LocalDespawn();
	}
}
void ACalibratioActor::LocalDespawn()
{
	GetWorld()->DestroyActor(this); // Destroy ourself
}

void ACalibratioActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* Search for tracker */
	if(!TrackedClusterComponent)
	{
		TrackedClusterComponent = UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_CALIBRATIO);
	} else
	{
		const FVector NewPos = TrackedClusterComponent->GetComponentLocation();
		if(NewPos != LastVisiblePosition)
		{
			if(FirstPositionSet) LastTimeVisible = FDateTime::Now();

			LastVisiblePosition = NewPos;
			FirstPositionSet = true;
		} 
	}

	if (!IsTrackerCurrentlyVisible() || !TrackedClusterComponent)
	{
		Overlay->SetPhysicalStatus(NotFound);
		Overlay->SetStatus(Waiting);
		return;
	}

	/* Tracker is visible */
	const float Rotation = TrackedClusterComponent->GetRelativeRotation().Yaw;

	/* First run, place mesh */
	if (CurrentCalibrationRuns == 0)
	{
		Mesh->SetRelativeLocation(TrackedClusterComponent->GetRelativeLocation());
	}
	/* More calibration runs to go */
	if (CurrentCalibrationRuns < MaxCalibrationRuns)
	{
		Overlay->SetStatus(Calibrating);
		if (IsDeviceMoving(Rotation))
		{
			Overlay->SetPhysicalStatus(Moving);
			LocalReset();
		}
		else
		{
			Overlay->SetPhysicalStatus(Found);
			CalibrationRun(Rotation);
		}
	}
	/* Calibration finished */
	else if (CurrentCalibrationRuns == MaxCalibrationRuns)
	{
		ClusterArmAndSetCalibration(MinRotation, MaxRotation); /* Sync to other nodes */
	}
	/* Actual Measuring */
	else if (Rotation < MinRotation - Threshold || Rotation > MaxRotation + Threshold)
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(ResetTimerHandle))
		{
			DynamicMaterial->SetVectorParameterValue("Color", FColor::White);
			Overlay->SetStatus(Triggered);
			GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, this, &ACalibratioActor::ClusterReset, ResetTime, false);
		}
	}
}

/* Checks if last 10 calls to this function handed in roughly the same Angle */
bool ACalibratioActor::IsDeviceMoving(float Angle)
{
	LastRotations.Add(Angle); /* Push into back */
	if (LastRotations.Num() > 10) LastRotations.RemoveAt(0); /* Remove first one */
	
	float Sum = 0.0f;
	for (float& CurrentAngle : LastRotations)
	{
		Sum += CurrentAngle;
	}
	const float Average = Sum / LastRotations.Num();
	
	return FMath::Abs(Average - Angle) > 5 * Threshold && LastRotations.Num() >= 10;
}

/* Adjusts min and max rotation values */
void ACalibratioActor::CalibrationRun(float Angle)
{
	if(!UVirtualRealityUtilities::IsMaster()) return; //Not our Business
	
	MinRotation = FMath::Min(MinRotation, Angle);
	MaxRotation = FMath::Max(MaxRotation, Angle);
	Overlay->SetThresholds(MinRotation, MaxRotation, Threshold);
	
	CurrentCalibrationRuns++;
}

bool ACalibratioActor::IsTrackerCurrentlyVisible()
{
	return (FDateTime::Now() - LastTimeVisible).GetTotalMilliseconds() <= AcceptedAbscenceTime;
}
