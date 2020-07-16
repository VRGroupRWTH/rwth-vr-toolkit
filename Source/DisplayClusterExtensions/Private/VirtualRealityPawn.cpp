#include "VirtualRealityPawn.h"

#include "Camera/CameraComponent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "IDisplayCluster.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h" // include draw debug helpers header file
#include "Math/Vector.h"
#include "VirtualRealityUtilities.h"


AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->UpdatedComponent = RootComponent;
	RotatingMovement->bRotationInLocalSpace = false;
	RotatingMovement->PivotTranslation = FVector::ZeroVector;
	RotatingMovement->RotationRate = FRotator::ZeroRotator;

	Head = CreateDefaultSubobject<USceneComponent>(TEXT("Head"));
	RightHand = CreateDefaultSubobject<USceneComponent>(TEXT("RightHand"));
	LeftHand = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));

	HmdLeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdLeftMotionController"));
	HmdLeftMotionController->SetupAttachment(RootComponent);
	HmdLeftMotionController->SetTrackingSource(EControllerHand::Left);
	HmdLeftMotionController->SetShowDeviceModel(true);
	HmdLeftMotionController->SetVisibility(false);

	HmdRightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdRightMotionController"));
	HmdRightMotionController->SetupAttachment(RootComponent);
	HmdRightMotionController->SetTrackingSource(EControllerHand::Right);
	HmdRightMotionController->SetShowDeviceModel(true);
	HmdRightMotionController->SetVisibility(false);

	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->OnComponentHit.AddDynamic(this, &AVirtualRealityPawn::OnHit);
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetupAttachment(CameraComponent);
	CapsuleColliderComponent->SetCapsuleSize(40.0f, 96.0f);
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	VRWolkingMode(Value, RightHand->GetForwardVector());

}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	VRWolkingMode(Value, RightHand->GetRightVector());
}

void AVirtualRealityPawn::OnTurnRate_Implementation(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate)
{
	if (UVirtualRealityUtilities::IsRoomMountedMode())
	{
		// User-centered projection causes simulation sickness on look up interaction hence not implemented.
	}
	else
	{
		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	}
}

float AVirtualRealityPawn::GetBaseTurnRate() const
{
	return BaseTurnRate;
}

void AVirtualRealityPawn::SetBaseTurnRate(float Value)
{
	BaseTurnRate = Value;
}

UFloatingPawnMovement* AVirtualRealityPawn::GetFloatingPawnMovement()
{
	return Movement;
}

URotatingMovementComponent* AVirtualRealityPawn::GetRotatingMovementComponent()
{
	return RotatingMovement;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetFlystickComponent()
{
	return Flystick;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetRightHandtargetComponent()
{
	return RightHandTarget;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetLeftHandtargetComponent()
{
	return LeftHandTarget;
}

UMotionControllerComponent* AVirtualRealityPawn::GetHmdLeftMotionControllerComponent()
{
	return HmdLeftMotionController;
}

UMotionControllerComponent* AVirtualRealityPawn::GetHmdRightMotionControllerComponent()
{
	return HmdRightMotionController;
}

USceneComponent* AVirtualRealityPawn::GetHeadComponent()
{
	return Head;
}

USceneComponent* AVirtualRealityPawn::GetLeftHandComponent()
{
	return LeftHand;
}

USceneComponent* AVirtualRealityPawn::GetRightHandComponent()
{
	return RightHand;
}

USceneComponent* AVirtualRealityPawn::GetTrackingOriginComponent()
{
	return TrackingOrigin;
}

USceneComponent* AVirtualRealityPawn::GetCaveCenterComponent()
{
	return CaveCenter;
}

USceneComponent* AVirtualRealityPawn::GetShutterGlassesComponent()
{
	return ShutterGlasses;
}

void AVirtualRealityPawn::ClusterExecute(const FString& Command)
{
	FDisplayClusterClusterEvent event;
	event.Name = "NDisplayCMD: " + Command;
	event.Type = "NDisplayCMD";
	event.Category = "VRPawn";
	event.Parameters.Add("Command", Command);
	IDisplayCluster::Get().GetClusterMgr()->EmitClusterEvent(event, false);
}

void AVirtualRealityPawn::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself. 
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComponent != nullptr))
	{
	
	}
}

void AVirtualRealityPawn::HandleClusterEvent(const FDisplayClusterClusterEvent& Event)
{
	if (Event.Category.Equals("VRPawn") && Event.Type.Equals("NDisplayCMD") && Event.Parameters.Contains("Command"))
	{
		GEngine->Exec(GetWorld(), *Event.Parameters["Command"]);
	}
}

void AVirtualRealityPawn::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//if (OtherActor && (OtherActor != this) && OtherComp)
	//{
	//	HasContact = true;
	//	DistancBetwCollisionAndClossestPointOnSurface = OtherComp->GetDistanceToCollision(SphereCollisionComponent->GetComponentLocation(), closestPointOnSurface);//Gibt die Entfernung zur nchstgelegenen Krperinstanzoberflche zurck.
	//}
}

void AVirtualRealityPawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	//if (OtherActor && (OtherActor != this) && OtherComp)
	//{
	//	HasContact = false;
	//}
}

void AVirtualRealityPawn::BeginPlay()
{
	Super::BeginPlay();

	// Display cluster settings apply to all setups (PC, HMD, CAVE/ROLV) despite the unfortunate name due to being an UE4 internal.
	TArray<AActor*> SettingsActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADisplayClusterSettings::StaticClass(), SettingsActors);
	if (SettingsActors.Num() > 0)
	{
		ADisplayClusterSettings* Settings = Cast<ADisplayClusterSettings>(SettingsActors[0]);
		Movement->MaxSpeed = Settings->MovementMaxSpeed;
		Movement->Acceleration = Settings->MovementAcceleration;
		Movement->Deceleration = Settings->MovementDeceleration;
		Movement->TurningBoost = Settings->MovementTurningBoost;
		BaseTurnRate = Settings->RotationSpeed;
	}

	if (UVirtualRealityUtilities::IsRoomMountedMode())
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		InitRoomMountedComponentReferences();
	}
	else if (UVirtualRealityUtilities::IsHeadMountedMode())
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		HmdLeftMotionController->SetVisibility(ShowHMDControllers);
		HmdRightMotionController->SetVisibility(ShowHMDControllers);

		LeftHand->AttachToComponent(HmdLeftMotionController, FAttachmentTransformRules::KeepRelativeTransform);
		RightHand->AttachToComponent(HmdRightMotionController, FAttachmentTransformRules::KeepRelativeTransform);
		Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	else //Desktop
	{
		Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		//also attach the hands to the camera component so we can use them for interaction
		LeftHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		RightHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);


		//move to eyelevel
		GetCameraComponent()->SetRelativeLocation(FVector(0, 0, 160));
	}

	//In ADisplayClusterPawn::BeginPlay() input is disabled on all slaves, so we cannot react to button presses, e.g. on the flystick correctly.
	//Therefore, we activate it again:
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			this->EnableInput(PlayerController);
		}
	}

	// Register cluster event listeners
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && !ClusterEventListenerDelegate.IsBound())
	{
		ClusterEventListenerDelegate = FOnClusterEventListener::CreateUObject(this, &AVirtualRealityPawn::HandleClusterEvent);
		ClusterManager->AddClusterEventListener(ClusterEventListenerDelegate);
	}

	CollisionComponent->SetCollisionProfileName(FName("NoCollision"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LastCameraPosition = CameraComponent->GetComponentLocation();
}

void AVirtualRealityPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
	if (ClusterManager && ClusterEventListenerDelegate.IsBound())
	{
		ClusterManager->RemoveClusterEventListener(ClusterEventListenerDelegate);
	}

	Super::EndPlay(EndPlayReason);
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DeltaTime = DeltaSeconds;

	SetCapsuleColliderCharacterSizeVR();
	if (IsColliderOnGround()) {
		VRClimbStepUp(DeltaSeconds);
		PhysWolkingMode();
	}



	//Flystick might not be available at start, hence is checked every frame.
	InitRoomMountedComponentReferences();

	LastCameraPosition = CameraComponent->GetComponentLocation();
}

void AVirtualRealityPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAxis("MoveForward", this, &AVirtualRealityPawn::OnForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AVirtualRealityPawn::OnRight);
		PlayerInputComponent->BindAxis("TurnRate", this, &AVirtualRealityPawn::OnTurnRate);
		PlayerInputComponent->BindAxis("LookUpRate", this, &AVirtualRealityPawn::OnLookUpRate);
	}
}

UPawnMovementComponent* AVirtualRealityPawn::GetMovementComponent() const
{
	return Movement;
}

void AVirtualRealityPawn::SetCapsuleColliderCharacterSizeVR()
{
	float CharachterSize = abs(RootComponent->GetComponentLocation().Z - CameraComponent->GetComponentLocation().Z);

	if (CharachterSize > MaxStepHeight)
	{
		float ColliderHeight = CharachterSize - MaxStepHeight;
		float ColliderHalfHeight = ColliderHeight / 2.0f;
		if (ColliderHalfHeight < 40.0f) {
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else {
			CapsuleColliderComponent->SetCapsuleSize(40.0f, ColliderHalfHeight);
		}

		CapsuleColliderComponent->SetWorldLocation(CameraComponent->GetComponentLocation());
		CapsuleColliderComponent->AddWorldOffset(FVector(0, 0, -ColliderHalfHeight));
		CapsuleColliderComponent->SetWorldRotation(FRotator(0, 0, 1));
	}
	else
	{
		CapsuleColliderComponent->SetWorldLocation(CameraComponent->GetComponentLocation());
		CapsuleColliderComponent->SetWorldRotation(FRotator(0, 0, 1));
	}
}

void AVirtualRealityPawn::PhysWolkingMode()
{
	FVector CurrentCameraPosition = CameraComponent->GetComponentLocation();
	FVector Direction = CurrentCameraPosition - LastCameraPosition;
	Direction.Z = 0.0f;
	FHitResult FHitResultPhys;
	CapsuleColliderComponent->AddWorldOffset(Direction, true, &FHitResultPhys);

	if (FHitResultPhys.bBlockingHit) {
		RootComponent->AddLocalOffset(FHitResultPhys.Normal*FHitResultPhys.PenetrationDepth);
	}
}

void AVirtualRealityPawn::VRWolkingMode(float Value, FVector Direction)
{
	Direction.Z = 0.0f;//Not falling/flying
	FVector End = (Direction * GetFloatingPawnMovement()->GetMaxSpeed());
	FHitResult FHitResultVR;
	CapsuleColliderComponent->AddWorldOffset(End* DeltaTime*Value, true, &FHitResultVR);

	if (FVector::Distance(FHitResultVR.Location, CapsuleColliderComponent->GetComponentLocation()) > CapsuleColliderComponent->GetScaledCapsuleRadius() && RightHand && (NavigationMode == EVRNavigationModes::nav_mode_walk || UVirtualRealityUtilities::IsDesktopMode() || UVirtualRealityUtilities::IsHeadMountedMode() || UVirtualRealityUtilities::IsRoomMountedMode()))
	{
		AddMovementInput(Direction, Value);

	}
}

bool AVirtualRealityPawn::IsColliderOnGround()
{
	float CharachterSize = abs(RootComponent->GetComponentLocation().Z - CameraComponent->GetComponentLocation().Z);
	if (CharachterSize > MaxStepHeight)
	{
		return true;
	}
	return false;
}

void AVirtualRealityPawn::VRClimbStepUp(float DeltaSeconds)
{
	FVector StartLineTraceUnderCollider = CapsuleColliderComponent->GetComponentLocation();
	StartLineTraceUnderCollider.Z -= CapsuleColliderComponent->GetScaledCapsuleHalfHeight();
	FHitResult HitDetailsMultiLineTrace = CreateMultiLineTrace(FVector(0, 0, -1), StartLineTraceUnderCollider, CapsuleColliderComponent->GetScaledCapsuleRadius() / 2.0f, true);

	if (HitDetailsMultiLineTrace.bBlockingHit && (HitDetailsMultiLineTrace.Distance < MaxStepHeight) && HitDetailsMultiLineTrace.Actor != RootComponent->GetAttachmentRootActor() && HitDetailsMultiLineTrace.Location != HitDetailsMultiLineTrace.TraceStart)
	{
		RootComponent->AddLocalOffset(FVector(0, 0, +abs(MaxStepHeight - HitDetailsMultiLineTrace.Distance)));
	}
	else if ((HitDetailsMultiLineTrace.Distance > MaxStepHeight))
	{
		RootComponent->AddLocalOffset(FVector(0, 0, -abs(MaxStepHeight - HitDetailsMultiLineTrace.Distance)));
	}
}

void AVirtualRealityPawn::InitRoomMountedComponentReferences()
{
	if (!UVirtualRealityUtilities::IsRoomMountedMode()) return;

	//check whether the nodes already exist (otherwise GetClusterComponent() returns nullptr and prints a warning) and assign them
	if (!TrackingOrigin) TrackingOrigin = UVirtualRealityUtilities::GetClusterComponent("cave_origin");
	if (!TrackingOrigin) TrackingOrigin = UVirtualRealityUtilities::GetClusterComponent("rolv_origin");
	if (!CaveCenter) CaveCenter = UVirtualRealityUtilities::GetClusterComponent("cave_center");
	if (!ShutterGlasses)
	{
		ShutterGlasses = UVirtualRealityUtilities::GetClusterComponent("shutter_glasses");
		Head->AttachToComponent(ShutterGlasses, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (!Flystick)
	{
		Flystick = UVirtualRealityUtilities::GetClusterComponent("flystick");
		if (AttachRightHandInCAVE == EAttachementType::AT_FLYSTICK)
			RightHand->AttachToComponent(Flystick, FAttachmentTransformRules::KeepRelativeTransform);
		if (AttachLeftHandInCAVE == EAttachementType::AT_FLYSTICK)
			LeftHand->AttachToComponent(Flystick, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (!LeftHandTarget)
	{
		LeftHandTarget = UVirtualRealityUtilities::GetClusterComponent("left_hand_target");
		if (AttachLeftHandInCAVE == EAttachementType::AT_HANDTARGET)
			LeftHand->AttachToComponent(LeftHandTarget, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (!RightHandTarget)
	{
		RightHandTarget = UVirtualRealityUtilities::GetClusterComponent("right_hand_target");
		if (AttachRightHandInCAVE == EAttachementType::AT_HANDTARGET)
			RightHand->AttachToComponent(RightHandTarget, FAttachmentTransformRules::KeepRelativeTransform);
	}
}


FHitResult AVirtualRealityPawn::CreateLineTrace(FVector Direction, const FVector Start, bool Visibility)
{
	//Re-initialize hit info
	FHitResult HitDetails = FHitResult(ForceInit);

	FVector End = ((Direction * 1000.f) + Start);
	// additional trace parameters
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.bTraceComplex = true; //to use complex collision on whatever we interact with to provide better precision.
	TraceParams.bReturnPhysicalMaterial = true; //to provide details about the physical material, if one exists on the thing we hit, to come back in our hit result.

	if (Visibility)
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(HitDetails, Start, End, ECC_Visibility, TraceParams))
	{
		if (HitDetails.bBlockingHit)
		{
		}
	}
	return HitDetails;
}

FHitResult AVirtualRealityPawn::CreateMultiLineTrace(FVector Direction, const FVector Start, float distance, bool Visibility) {
	FHitResult HitDetailsMultiLineTrace;

	FHitResult HitDetailsFromLineTraceCenter = CreateLineTrace(Direction, Start, Visibility);
	FHitResult HitDetailsFromLineTraceLeft = CreateLineTrace(Direction, Start + FVector(0, -distance, 0), Visibility);
	FHitResult HitDetailsFromLineTraceRight = CreateLineTrace(Direction, Start + FVector(0, distance, 0), Visibility);
	FHitResult HitDetailsFromLineTraceFront = CreateLineTrace(Direction, Start + FVector(distance, 0, 0), Visibility);
	FHitResult HitDetailsFromLineTraceBehind = CreateLineTrace(Direction, Start + FVector(-distance, 0, 0), Visibility);
	bool bBlockingHitAndSameActor = (HitDetailsFromLineTraceCenter.bBlockingHit && HitDetailsFromLineTraceLeft.bBlockingHit
		&& HitDetailsFromLineTraceRight.bBlockingHit && HitDetailsFromLineTraceFront.bBlockingHit
		&& HitDetailsFromLineTraceBehind.bBlockingHit)
		&& (HitDetailsFromLineTraceCenter.Actor == HitDetailsFromLineTraceLeft.Actor
		&& HitDetailsFromLineTraceLeft.Actor == HitDetailsFromLineTraceRight.Actor
		&& HitDetailsFromLineTraceRight.Actor == HitDetailsFromLineTraceFront.Actor
		&& HitDetailsFromLineTraceFront.Actor == HitDetailsFromLineTraceBehind.Actor);
	if (bBlockingHitAndSameActor)
		HitDetailsMultiLineTrace = HitDetailsFromLineTraceCenter;

	return HitDetailsMultiLineTrace;
}
