#include "VirtualRealityPawn.h"

#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "IDisplayCluster.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/Vector.h"
#include "VirtualRealityUtilities.h"

#include "GrabbingBehaviorComponent.h"
#include "Grabable.h"
#include "Clickable.h"


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
	Head->SetupAttachment(RootComponent);
	RightHand = CreateDefaultSubobject<USceneComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(RootComponent);
	LeftHand = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);

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
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetupAttachment(CameraComponent);
	CapsuleColliderComponent->SetCapsuleSize(40.0f, 96.0f);
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	if (RightHand)
	{
		HandleMovementInput(Value, RightHand->GetForwardVector());
	}
}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	if (RightHand)
	{
		HandleMovementInput(Value, RightHand->GetRightVector());
	}
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

void AVirtualRealityPawn::HandleClusterEvent(const FDisplayClusterClusterEvent& Event)
{
	if (Event.Category.Equals("VRPawn") && Event.Type.Equals("NDisplayCMD") && Event.Parameters.Contains("Command"))
	{
		GEngine->Exec(GetWorld(), *Event.Parameters["Command"]);
	}
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

		LeftHand->AttachToComponent(HmdLeftMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
		RightHand->AttachToComponent(HmdRightMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
		Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	else //Desktop
	{
		Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

		//also attach the hands to the camera component so we can use them for interaction
		LeftHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		RightHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);


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
	//if the walking-mode is activated
	if (NavigationMode == EVRNavigationModes::nav_mode_walk)
	{
		DeltaTime = DeltaSeconds;
		SetCapsuleColliderCharacterSizeVR();
		MoveByGravityOrStepUp(DeltaSeconds);
		CheckForPhysWalkingCollision();
	}

	// if an actor is grabbed and a behavior is defined move move him accordingly  
	if (GrabbedActor != nullptr)
	{
		UGrabbingBehaviorComponent* Behavior = GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>();

		// if our Grabable Actor is not constrained
		if (Behavior != nullptr)
		{	
			// specifies the hand in space
			FVector HandPos = this->RightHand->GetComponentLocation();	
			FQuat HandQuat = this->RightHand->GetComponentQuat();

			Behavior->HandleNewPositionAndDirection(HandPos, HandQuat); 
		}
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

		// function bindings for grabbing and releasing
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AVirtualRealityPawn::OnBeginFire);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &AVirtualRealityPawn::OnEndFire);
	}
}

void AVirtualRealityPawn::OnBeginFire_Implementation()
{
	// start and end point for raytracing
	FTwoVectors StartEnd = GetHandRay(MaxClickDistance);	
	FVector Start = StartEnd.v1;
	FVector End   = StartEnd.v2;	

	// will be filled by the Line Trace Function
	FHitResult Hit;
	AActor* HitActor;

	//if hit was not found return  
	FCollisionObjectQueryParams Params;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, Params))
		return;

//	UE_LOG(LogTemp, Warning, GetDisplayName(Hit.GetActor()));
	HitActor = Hit.GetActor();
	
	// try to cast HitActor int a Grabable if not succeeded will become a nullptr
	IGrabable*  GrabableActor  = Cast<IGrabable>(HitActor);
	IClickable* ClickableActor = Cast<IClickable>(HitActor);

	if (GrabableActor != nullptr && Hit.Distance < MaxGrabDistance)
	{
		// call grabable actors function so he reacts to our grab
		GrabableActor->OnGrabbed_Implementation();
		

		UGrabbingBehaviorComponent* Behavior = HitActor->FindComponentByClass<UGrabbingBehaviorComponent>();
		if ( Behavior == nullptr)
			HandlePhysicsAndAttachActor(HitActor);

		// we save the grabbedActor in a general form to access all of AActors functions easily later
		GrabbedActor = HitActor;
	}
	else if (ClickableActor != nullptr && Hit.Distance < MaxClickDistance)
	{
		ClickableActor->OnClicked_Implementation();
	}
}

void AVirtualRealityPawn::HandlePhysicsAndAttachActor(AActor* HitActor)
{
	UPrimitiveComponent* PhysicsComp = HitActor->FindComponentByClass<UPrimitiveComponent>();	
	
	bDidSimulatePhysics = PhysicsComp->IsSimulatingPhysics(); // remember if we need to tun physics back on or not	
	PhysicsComp->SetSimulatePhysics(false);
	FAttachmentTransformRules Rules = FAttachmentTransformRules::KeepWorldTransform;
	Rules.bWeldSimulatedBodies = true;
	HitActor->AttachToComponent(this->RightHand, Rules);
}

void AVirtualRealityPawn::OnEndFire_Implementation() {

	// if we didnt grab anyone there is no need to release
	if (GrabbedActor == nullptr)
		return;

	// let the grabbed object reacot to release
	Cast<IGrabable>(GrabbedActor)->OnReleased_Implementation();

	// Detach the Actor

	UPrimitiveComponent* PhysicsComp = GrabbedActor->FindComponentByClass<UPrimitiveComponent>();
	UGrabbingBehaviorComponent* Behavior = GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>();
	if (Behavior == nullptr)
	{
		GrabbedActor->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		PhysicsComp->SetSimulatePhysics(bDidSimulatePhysics);
	}

	// forget about the actor
	GrabbedActor = nullptr;
}

FTwoVectors AVirtualRealityPawn::GetHandRay(float Length)
{
	FVector Start = this->RightHand->GetComponentLocation();
	FVector Direction = this->RightHand->GetForwardVector();
	FVector End = Start + Length * Direction;

	return FTwoVectors(Start, End);
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
		float ColliderRadius = 40.0f;
		if (ColliderHalfHeight <= ColliderRadius)
		{//Make the collider to a Sphere
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else
		{//Make the collider to a Capsule
			CapsuleColliderComponent->SetCapsuleSize(ColliderRadius, ColliderHalfHeight);
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

void AVirtualRealityPawn::CheckForPhysWalkingCollision()
{
	FVector CurrentCameraPosition = CameraComponent->GetComponentLocation();
	FVector Direction = CurrentCameraPosition - LastCameraPosition;
	//Direction.Z = 0.0f;//walking
	FHitResult FHitResultPhys;
	CapsuleColliderComponent->AddWorldOffset(Direction, true, &FHitResultPhys);

	if (FHitResultPhys.bBlockingHit)
	{
		RootComponent->AddLocalOffset(FHitResultPhys.Normal*FHitResultPhys.PenetrationDepth);
	}
}

void AVirtualRealityPawn::HandleMovementInput(float Value, FVector Direction)
{
	if (NavigationMode == EVRNavigationModes::nav_mode_walk)
	{
		VRWalkingMode(Value, Direction);
	}
	else if (NavigationMode == EVRNavigationModes::nav_mode_fly)
	{
		VRFlyingMode(Value, Direction);
	}
}

void AVirtualRealityPawn::VRWalkingMode(float Value, FVector Direction)
{
	Direction.Z = 0.0f;//walking
	FVector End = (Direction * GetFloatingPawnMovement()->GetMaxSpeed());
	FHitResult FHitResultVR;
	CapsuleColliderComponent->AddWorldOffset(End* DeltaTime*Value, true, &FHitResultVR);

	if (FVector::Distance(FHitResultVR.Location, CapsuleColliderComponent->GetComponentLocation()) > CapsuleColliderComponent->GetScaledCapsuleRadius())
	{
		AddMovementInput(Direction, Value);
	}
}

void AVirtualRealityPawn::VRFlyingMode(float Value, FVector Direction)
{
	AddMovementInput(Direction, Value);
}

void AVirtualRealityPawn::MoveByGravityOrStepUp(float DeltaSeconds)
{
	FVector StartLineTraceUnderCollider = CapsuleColliderComponent->GetComponentLocation();
	StartLineTraceUnderCollider.Z -= CapsuleColliderComponent->GetScaledCapsuleHalfHeight();
	FHitResult HitDetailsMultiLineTrace = CreateMultiLineTrace(FVector(0, 0, -1), StartLineTraceUnderCollider, CapsuleColliderComponent->GetScaledCapsuleRadius() / 4.0f, true);
	float DiffernceDistance = abs(MaxStepHeight - HitDetailsMultiLineTrace.Distance);
	//Going up
	if ((HitDetailsMultiLineTrace.bBlockingHit && HitDetailsMultiLineTrace.Distance < MaxStepHeight))
	{
		ShiftVertically(DiffernceDistance, UpSteppingAcceleration, DeltaSeconds, 1);
	}
	//Falling, Gravity, Going down
	else if ((HitDetailsMultiLineTrace.bBlockingHit && HitDetailsMultiLineTrace.Distance > MaxStepHeight) || HitDetailsMultiLineTrace.Actor == nullptr && HitDetailsMultiLineTrace.Distance != -1.0f)
	{
		ShiftVertically(DiffernceDistance, GravityAcceleration, DeltaSeconds, -1);
	}
}

void AVirtualRealityPawn::ShiftVertically(float DiffernceDistance, float Acceleration, float DeltaSeconds, int Direction)
{
	VerticalSpeed += Acceleration * DeltaSeconds;
	if (VerticalSpeed*DeltaSeconds < DiffernceDistance)
	{
		RootComponent->AddLocalOffset(FVector(0.f, 0.f, Direction * VerticalSpeed * DeltaSeconds));
	}
	else
	{
		RootComponent->AddLocalOffset(FVector(0.f, 0.f, Direction * DiffernceDistance));
		VerticalSpeed = 0;
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
		Head->AttachToComponent(ShutterGlasses, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	if (!Flystick)
	{
		Flystick = UVirtualRealityUtilities::GetClusterComponent("flystick");
		if (AttachRightHandInCAVE == EAttachementType::AT_FLYSTICK)
			RightHand->AttachToComponent(Flystick, FAttachmentTransformRules::SnapToTargetIncludingScale);
		if (AttachLeftHandInCAVE == EAttachementType::AT_FLYSTICK)
			LeftHand->AttachToComponent(Flystick, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	if (!LeftHandTarget)
	{
		LeftHandTarget = UVirtualRealityUtilities::GetClusterComponent("left_hand_target");
		if (AttachLeftHandInCAVE == EAttachementType::AT_HANDTARGET)
			LeftHand->AttachToComponent(LeftHandTarget, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	if (!RightHandTarget)
	{
		RightHandTarget = UVirtualRealityUtilities::GetClusterComponent("right_hand_target");
		if (AttachRightHandInCAVE == EAttachementType::AT_HANDTARGET)
			RightHand->AttachToComponent(RightHandTarget, FAttachmentTransformRules::SnapToTargetIncludingScale);
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

FHitResult AVirtualRealityPawn::CreateMultiLineTrace(FVector Direction, const FVector Start, float Radius, bool Visibility)
{
	TArray<FVector> StartVectors;
	TArray<FHitResult> OutHits;
	FHitResult HitDetailsMultiLineTrace;
	HitDetailsMultiLineTrace.Distance = -1.0f;//(Distance=-1) not existing, but to know if this Variable not Initialized(when all Traces not compatible)

	StartVectors.Add(Start); //LineTraceCenter
	StartVectors.Add(Start + FVector(0, -Radius, 0)); //LineTraceLeft
	StartVectors.Add(Start + FVector(0, +Radius, 0)); //LineTraceRight
	StartVectors.Add(Start + FVector(+Radius, 0, 0)); //LineTraceFront
	StartVectors.Add(Start + FVector(-Radius, 0, 0)); //LineTraceBehind

	bool IsBlockingHitAndSameActor = true;
	bool IsAllNothingHiting = true;
	// loop through TArray
	for (FVector& Vector : StartVectors)
	{
		FHitResult OutHit = CreateLineTrace(Direction, Vector, Visibility);
		OutHits.Add(OutHit);
		IsBlockingHitAndSameActor &= (OutHit.Actor == OutHits[0].Actor); //If all Hiting the same Object, then you are (going up/down) or (walking)
		IsAllNothingHiting &= (OutHit.Actor == nullptr); //If all Hiting nothing, then you are falling
	}

	if (IsBlockingHitAndSameActor || IsAllNothingHiting)
		HitDetailsMultiLineTrace = OutHits[0];

	return HitDetailsMultiLineTrace;
}
