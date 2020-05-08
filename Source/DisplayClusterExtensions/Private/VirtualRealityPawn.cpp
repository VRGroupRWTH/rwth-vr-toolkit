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
	HasContact = false;

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

	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BaseSphereComponent"));
	SphereCollisionComponent->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	SphereCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AVirtualRealityPawn::OnOverlapBegin);
	SphereCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AVirtualRealityPawn::OnOverlapEnd);
	SphereCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->SetupAttachment(RootComponent);
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionProfileName("Pawn");

	// das hier ist nur da damit man sieht wo die Kapsel ist!
	CapsuleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	CapsuleMesh->SetupAttachment(CapsuleColliderComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	CapsuleMesh->SetStaticMesh(SphereMeshAsset.Object);
	CapsuleMesh->SetRelativeScale3D(FVector(0.1f));
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	FHitResult HitResults;
	CapsuleColliderComponent->AddRelativeLocation(5.0f*RightHand->GetForwardVector()*Value, true, &HitResults);

	if (HitResults.bBlockingHit) {
		UE_LOG(LogTemp, Warning, TEXT("Hit something"));
	}


	//so kann ich prfen ob es eine overlap gibt, dafr mssen aber alle Objekte in der Szene overlap events generieren, was normalerweise aus ist.
	TArray<AActor*> OverlappingActors;
	CapsuleColliderComponent->GetOverlappingActors(OverlappingActors);

	bool bOverlapping = false;
	for(AActor* other : OverlappingActors)
	{
		if (other == this)
			continue;
		UE_LOG(LogTemp, Warning, TEXT("Overlapping %s"), *other->GetName());
		bOverlapping = true;
	}

	if(!bOverlapping)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not overlapping"));
	}



	//FVector StartFromKnee = GetCameraComponent()->GetComponentLocation();
	//StartFromKnee.Z -= (DistBetwCameraAndGroundZ - MaxStepHeight);
	//FVector CurrentCameraPosition = GetCameraComponent()->GetComponentLocation();
	//FVector DirectionVector = CurrentCameraPosition - LastCameraPosition;
	//LineTraceData LineTraceDataWhereYouGoing = CreateLineTrace(FVector(DirectionVector.X, DirectionVector.Y, 0.f), StartFromKnee, true);
	////Moving the Pawn.
	//if (FVector::Distance(LineTraceDataWhereYouGoing.MyImpactPoint, StartFromKnee) <= SphereCollisionComponent->GetScaledSphereRadius())
	//{
	//	if (Value!=0.0f) {//Moving the Pawn, if you go into other objects with joystick/flystik/pawn.
	//		RootComponent->SetWorldLocation(LastPawnPosition, true);
	//	}
	//	else {//Moving the pawn, if you physically go into objects with the SphereCollisionComponent.
	//		FVector DiffImpactPointBellyForwardAndStartFromKnee = LineTraceDataWhereYouGoing.MyImpactPoint - StartFromKnee;
	//		float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - FVector::Distance(LineTraceDataWhereYouGoing.MyImpactPoint, StartFromKnee);
	//		RootComponent->AddLocalOffset(DiffImpactPointBellyForwardAndStartFromKnee.GetSafeNormal()*Inside_Distance, true);
	//	}
	//}
	//// Check if this function triggers correctly on ROLV.
	//if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || UVirtualRealityUtilities::IsDesktopMode() || UVirtualRealityUtilities::IsHeadMountedMode()))
	//{
	//
	//	AddMovementInput(RightHand->GetForwardVector(), Value);
	//}
	//else if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_walk || UVirtualRealityUtilities::IsDesktopMode() || UVirtualRealityUtilities::IsHeadMountedMode() || UVirtualRealityUtilities::IsRoomMountedMode()))
	//{
	//	AddMovementInput(RightHand->GetForwardVector(), Value);
	//}

	LastCameraPosition = GetCameraComponent()->GetComponentLocation();
	LastPawnPosition = GetRootComponent()->GetComponentLocation();
}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || UVirtualRealityUtilities::IsDesktopMode() || UVirtualRealityUtilities::IsHeadMountedMode()))
	{
		AddMovementInput(RightHand->GetRightVector(), Value);
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

void AVirtualRealityPawn::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
       HasContact = true;
	   DistancBetwCollisionAndClossestPointOnSurface = OtherComp->GetDistanceToCollision(SphereCollisionComponent->GetComponentLocation(), closestPointOnSurface);//Gibt die Entfernung zur nächstgelegenen Körperinstanzoberfläche zurück.
	 }
}

void AVirtualRealityPawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		HasContact = false;
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

	LastCameraPosition = GetCameraComponent()->GetComponentLocation();
	LastPawnPosition = GetRootComponent()->GetComponentLocation();
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

	LineTraceData LineTraceDataObjekt = CreateLineTrace(FVector(0, 0, -1), GetCameraComponent()->GetComponentLocation(), false);
	DistBetwCameraAndGroundZ = abs(LineTraceDataObjekt.MyImpactPoint.Z - GetCameraComponent()->GetComponentLocation().Z);
	float DistBetwCameraAndPawnZ = abs(RootComponent->GetComponentLocation().Z - GetCameraComponent()->GetComponentLocation().Z);
	float DiffernceDistance = abs(DistBetwCameraAndGroundZ - DistBetwCameraAndPawnZ);
	static float SumUpSteppingSpeedWithDeltaSeconds = 0.0f;
	if (DistBetwCameraAndGroundZ < DistBetwCameraAndPawnZ)
	{
		SumUpSteppingSpeedWithDeltaSeconds += UpSteppingSpeed*DeltaSeconds;
		if (SumUpSteppingSpeedWithDeltaSeconds*DeltaSeconds < DiffernceDistance)
		{
			RootComponent->AddLocalOffset(FVector(0.f, 0.f, SumUpSteppingSpeedWithDeltaSeconds*DeltaSeconds), true);
		}
		else
		{
			RootComponent->AddLocalOffset(FVector(0.f, 0.f, DiffernceDistance), true);
		}
	}
	else if (DistBetwCameraAndGroundZ > DistBetwCameraAndPawnZ || !LineTraceDataObjekt.IsHit)
	{
		GravitySpeed -= 80000.0f*DeltaSeconds;
		if (GravitySpeed*DeltaSeconds > -DiffernceDistance || !LineTraceDataObjekt.IsHit)
		{
			RootComponent->AddLocalOffset(FVector(0.f, 0.f, GravitySpeed*DeltaSeconds), true);
		}
		else
		{
			RootComponent->AddLocalOffset(FVector(0.f, 0.f, -DiffernceDistance), true);
		}
	}
	else
	{
		GravitySpeed = 0.0f;
		SumUpSteppingSpeedWithDeltaSeconds = 0.0f;
	}

	//Verschieben des Pawns, wenn man pyisikalisch mit der Kollision-Spere der Camera reingeht.
	if (HasContact && NavigationMode== EVRNavigationModes::nav_mode_walk)
	{
	   FVector Diff_Camera_and_ClosestPointOnSurface = SphereCollisionComponent->GetComponentLocation() - closestPointOnSurface;
	   
	   float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - DistancBetwCollisionAndClossestPointOnSurface;

	   RootComponent->AddLocalOffset(Diff_Camera_and_ClosestPointOnSurface.GetSafeNormal()*Inside_Distance, true);
	}

	//Flystick might not be available at start, hence is checked every frame.
	InitRoomMountedComponentReferences();
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


 AVirtualRealityPawn::LineTraceData AVirtualRealityPawn::CreateLineTrace(FVector Direction, const FVector Start, bool Visibility)
{
	LineTraceData MyLineTraceData{ false, FVector(0,0,0) };
	{ //LineTrace 

		FHitResult OutHit;

		FVector End = ((Direction * 1000.f) + Start);
		FCollisionQueryParams CollisionParams;

		if(Visibility)
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				MyLineTraceData.MyImpactPoint = OutHit.ImpactPoint;
				MyLineTraceData.IsHit = true;
			}
		}
	}
	return MyLineTraceData;
}





