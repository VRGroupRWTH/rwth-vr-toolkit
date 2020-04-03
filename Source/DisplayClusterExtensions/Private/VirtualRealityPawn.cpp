#include "VirtualRealityPawn.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "IDisplayCluster.h"

#include "IDisplayClusterConfigManager.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h" // include draw debug helpers header file
#include "Math/Vector.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;

	OnForwardClicked = false;
	OnRightClicked = false;
	
	HasContact = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->UpdatedComponent = RootComponent;
	RotatingMovement->bRotationInLocalSpace = false;
	RotatingMovement->PivotTranslation = FVector::ZeroVector;
	RotatingMovement->RotationRate = FRotator::ZeroRotator;

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

}


void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	
	//Verschieben des Pawns, wenn man pyisikalisch mit der Kollision-Spere der Camera reingeht.
	AktualyPawnPosition = GetRootComponent()->GetComponentLocation();
	AktualyCameraPosition = GetCameraComponent()->GetComponentLocation();
	FVector Richtungsvektor = AktualyCameraPosition - LastCameraPosition;
	FVector ImpactPointWohinIchGehst = CreateLineTrace(FVector(Richtungsvektor.X, Richtungsvektor.Y, 0.f), StartFromKnee,false);

	if (Richtungsvektor.Size() > SphereCollisionComponent->GetScaledSphereRadius())
		UE_LOG(LogTemp, Warning, TEXT(" Richtungsvektor:                            %f "), Richtungsvektor.Size());

	if (FVector::Distance(ImpactPointWohinIchGehst, StartFromKnee) <= SphereCollisionComponent->GetScaledSphereRadius()) {
		if (OnForwardClicked) {//Verschieben des Pawns, wenn man mit der Jojstik/Flystik/Pawn in andere Gegenstaende reingeht.
			RootComponent->SetWorldLocation(LastPawnPosition, true);
		}
		else {//Verschieben des Pawns, wenn man pyisikalisch mit der Kollision-Spere der Camera reingeht.
			FVector DiffImpactPointBellyForwardAndStartFromKnee = ImpactPointWohinIchGehst - StartFromKnee;
			float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - FVector::Distance(ImpactPointWohinIchGehst, StartFromKnee);
			RootComponent->AddLocalOffset(DiffImpactPointBellyForwardAndStartFromKnee.GetSafeNormal()*Inside_Distance, true);
		}
	}



	if(Value !=0)
	OnForwardClicked = true;
	else
	OnForwardClicked = false;

	// Check if this function triggers correctly on ROLV.
	if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || IsDesktopMode()))
	{
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}
	else if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_walk || IsDesktopMode())) 
	{
		
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}


	{//Gravity
		FVector ImpactPoint_Down = CreateLineTrace(FVector(0, 0, -1), GetCameraComponent()->GetComponentLocation(), false);
		float DistBetwCameraAndGroundZ = abs(ImpactPoint_Down.Z - GetCameraComponent()->GetComponentLocation().Z);
		float DistBetwCameraAndPawnZ = abs(RootComponent->GetComponentLocation().Z - GetCameraComponent()->GetComponentLocation().Z);
		float DiffernceDistance = abs(DistBetwCameraAndGroundZ - DistBetwCameraAndPawnZ);
		

		{//Nicht den Hocker hochgehen.
			float Stufenhoehe_cm =45.f;
		    StartFromKnee = FVector(GetCameraComponent()->GetComponentLocation().X, GetCameraComponent()->GetComponentLocation().Y, GetCameraComponent()->GetComponentLocation().Z - (DistBetwCameraAndGroundZ - Stufenhoehe_cm));
		}
		

		//if you not have ImpactPoint, then you are falling.
		if (ImpactPoint_Down.Size() == 0.f) {
			static float GravitySpeed = 0.0;
			GravitySpeed += 0.05;
			FVector GravityAcc = FVector(0.f, 0.f, -1.f);
			const FVector LocalMove = FVector(0.f, 0.f, 0.f) + GravityAcc;
			RootComponent->AddLocalOffset(LocalMove*GravitySpeed, true);
		}
		//Treppe hochgehen/runtergehen.
		else {
			const FVector LocalUpMove{ 0.f, 0.f, +DiffernceDistance };

            if (DistBetwCameraAndGroundZ < DistBetwCameraAndPawnZ) {
				LocalUpMove{ 0.f, 0.f, +DiffernceDistance };
				RootComponent->AddLocalOffset(LocalUpMove, true);
			}
			else if (DistBetwCameraAndGroundZ > DistBetwCameraAndPawnZ) {
				LocalUpMove{ 0.f, 0.f, -DiffernceDistance };
				RootComponent->AddLocalOffset(LocalUpMove, true);
			}
		}
	}

	LastCameraPosition = GetCameraComponent()->GetComponentLocation();
	LastPawnPosition = GetRootComponent()->GetComponentLocation();

}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	if (Value != 0)
		OnRightClicked = true;
	else
		OnRightClicked = false;
	
	if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || IsDesktopMode()))
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
	AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);

}

void AVirtualRealityPawn::OnFire_Implementation(bool Pressed)
{
}

void AVirtualRealityPawn::OnAction_Implementation(bool Pressed, int32 Index)
{
}

bool AVirtualRealityPawn::IsDesktopMode()
{
	return !IsRoomMountedMode() && !IsHeadMountedMode();
}

bool AVirtualRealityPawn::IsRoomMountedMode()
{
	return IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster;
}

bool AVirtualRealityPawn::IsHeadMountedMode()
{
	return GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
}

FString AVirtualRealityPawn::GetNodeName()
{
	return IsRoomMountedMode() ? IDisplayCluster::Get().GetClusterMgr()->GetNodeId() : FString(TEXT("Localhost"));
}

float AVirtualRealityPawn::GetEyeDistance()
{
	return IDisplayCluster::Get().GetConfigMgr()->GetConfigStereo().EyeDist;
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

USceneComponent* AVirtualRealityPawn::GetCaveOriginComponent()
{
	return CaveOrigin;
}

USceneComponent* AVirtualRealityPawn::GetCaveCenterComponent()
{
	return CaveCenter;
}

USceneComponent* AVirtualRealityPawn::GetShutterGlassesComponent()
{
	return ShutterGlasses;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetClusterComponent(const FString& Name)
{
	return IDisplayCluster::Get().GetGameMgr()->GetNodeById(Name);
}

void AVirtualRealityPawn::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

	if (OtherActor && (OtherActor != this) && OtherComp) {
       HasContact = true;
	   Point = SphereCollisionComponent->GetComponentLocation(); //World 3D vector
	   dist_Betw_Collision_And_ClossestPointOnSurface = OtherComp->GetDistanceToCollision(Point, closestPointOnSurface);//Gibt die Entfernung zur nächstgelegenen Körperinstanzoberfläche zurück.
	
	 }
	
}

void AVirtualRealityPawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{

	if (OtherActor && (OtherActor != this) && OtherComp) {
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

	if (IsRoomMountedMode()) //Cave
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		InitComponentReferences();

		RootComponent->SetWorldLocation(FVector(0, 2, 0), false, nullptr, ETeleportType::None);
	}
	else if (IsHeadMountedMode()) //HMD
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		HmdLeftMotionController->SetVisibility(true);
		HmdRightMotionController->SetVisibility(true);

		LeftHand = HmdLeftMotionController;
		RightHand = HmdRightMotionController;
		Head = GetCameraComponent();
	}
	else //Desktop
	{
		LeftHand = RootComponent;
		RightHand = RootComponent;
		Head = GetCameraComponent();
	}

	CollisionComponent->SetCollisionProfileName(FName("NoCollision"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LastCameraPosition = GetCameraComponent()->GetComponentLocation();
	LastPawnPosition = GetRootComponent()->GetComponentLocation();
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Flystick might not be available at start, hence is checked every frame.
	InitComponentReferences();
	

	
	//Verschieben des Pawns, wenn man pyisikalisch mit der Kollision-Spere der Camera reingeht.
	if (HasContact && NavigationMode== EVRNavigationModes::nav_mode_walk) {

	   FVector Diff_Camera_and_ClosestPointOnSurface = Point - closestPointOnSurface;
	   
	   float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - dist_Betw_Collision_And_ClossestPointOnSurface;

	   RootComponent->AddLocalOffset(Diff_Camera_and_ClosestPointOnSurface.GetSafeNormal()*Inside_Distance, true);
	}

	
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

void AVirtualRealityPawn::InitComponentReferences()
{
	if (!IsRoomMountedMode()) return;
	if (!CaveOrigin) CaveOrigin = GetClusterComponent("cave_origin");
	if (!CaveCenter) CaveCenter = GetClusterComponent("cave_center");
	if (!ShutterGlasses)
	{
		ShutterGlasses = GetClusterComponent("shutter_glasses");
		Head = ShutterGlasses;
	}
	if (!Flystick)
	{
		Flystick = GetClusterComponent("flystick");

		LeftHand = Flystick;
		RightHand = Flystick;
	}
}


FVector AVirtualRealityPawn::CreateLineTrace(FVector Forward_Right_OR_Up_Vector, const FVector MyObjekt_ComponentLocation, bool Visibility)
{
	FVector MyImpactPoint{0.0f, 0.0f, 0.0f};
	{ //LineTrace 

		FHitResult OutHit;
		FVector Start = MyObjekt_ComponentLocation;

		FVector End = ((Forward_Right_OR_Up_Vector * 1000.f) + Start);
		FCollisionQueryParams CollisionParams;

		if(Visibility)
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				 MyImpactPoint = OutHit.ImpactPoint;
			}

		}
	}
	return MyImpactPoint;
}



EEyeType AVirtualRealityPawn::GetNodeEyeType() {
	FDisplayClusterConfigClusterNode CurrentNodeConfig;
	IDisplayCluster::Get().GetConfigMgr()->GetClusterNode(GetNodeName(), CurrentNodeConfig);

	FString s = CurrentNodeConfig.ToString();

	if (s.Contains("mono_eye")) {
		TArray<FString> stringArray;
		int32 count = s.ParseIntoArray(stringArray, TEXT(","));
		for (int x = 0; x < count; x++) {
			if (!stringArray[x].Contains("mono_eye")) continue;
			if (stringArray[x].Contains("left")) {
				return EEyeType::ET_STEREO_LEFT;
			}
			if (stringArray[x].Contains("right")) {
				return EEyeType::ET_STEREO_RIGHT;
			}
		}
	}
	else {
		return EEyeType::ET_MONO;
	}
	return EEyeType::ET_MONO;
}
