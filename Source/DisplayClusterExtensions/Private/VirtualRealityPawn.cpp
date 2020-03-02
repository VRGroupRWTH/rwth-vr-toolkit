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


	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BaseCapsuleComponent"));
	SphereCollisionComponent->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);


	SphereCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AVirtualRealityPawn::OnOverlapBegin);
	SphereCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AVirtualRealityPawn::OnOverlapEnd);
	SphereCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{

	bool isDistSmallerThenRadiusCollision_ForwardVector = CreateLineTrace(Value, GetCameraComponent()->GetForwardVector());
	bool isDistSmallerThenRadiusCollision_RightVector = CreateLineTrace(Value, GetCameraComponent()->GetRightVector());
	bool isDistSmallerThenRadiusCollision_LeftVector = CreateLineTrace(Value, -GetCameraComponent()->GetRightVector());


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
		if ((isDistSmallerThenRadiusCollision_ForwardVector || isDistSmallerThenRadiusCollision_RightVector || isDistSmallerThenRadiusCollision_LeftVector)  && Value > 0.0f ) {
			Value = 0;
		}
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}

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
	//if (IsRoomMountedMode())
	//{
	//	//const FVector CameraLocation = IDisplayCluster::Get().GetGameMgr()->GetActiveCamera()->GetComponentLocation();
	//	//RotatingMovement->PivotTranslation = RotatingMovement
	//	//                                     ->UpdatedComponent->GetComponentTransform().
	//	//                                     InverseTransformPositionNoScale(CameraLocation);
	//	//RotatingMovement->RotationRate = FRotator(RotatingMovement->RotationRate.Pitch, Rate * BaseTurnRate, 0.0f);

	//}
	//else
	//{
	//	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	//}
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate)
{
	//if (IsHeadMountedMode() && NavigationMode == EVRNavigationModes::nav_mode_walk)
	//{
	//
	//}
	//else
	//{


		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	//}
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
       GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap Begin"));
       UE_LOG(LogTemp, Warning, TEXT("Overlap Begin"));
       HasContact = true;
	   Point = SphereCollisionComponent->GetComponentLocation(); //World 3D vector
	   dist_Betw_Collision_And_ClossestPointOnSurface = OtherComp->GetDistanceToCollision(Point, closestPointOnSurface);//Gibt die Entfernung zur nächstgelegenen Körperinstanzoberfläche zurück.
	
	 }   
	
}

void AVirtualRealityPawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{

	if (OtherActor && (OtherActor != this) && OtherComp) {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap End"));
	    UE_LOG(LogTemp, Warning, TEXT("Overlap End"));
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

	if (NavigationMode == EVRNavigationModes::nav_mode_walk || NavigationMode == EVRNavigationModes::nav_mode_none) {
	   CollisionComponent->SetCollisionProfileName(FName("BlockAll"));
	   CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	if (NavigationMode == EVRNavigationModes::nav_mode_fly) {
		CollisionComponent->SetCollisionProfileName(FName("NoCollision"));
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//CollisionComponent->SetCollisionProfileName(FName("BlockAll"));
	//CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//BaseCollisionComponent->SetCollisionProfileName(FName("BlockAll"));
	//BaseCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Flystick might not be available at start, hence is checked every frame.
	InitComponentReferences();
	
	//if (NavigationMode == EVRNavigationModes::nav_mode_walk)
	//    UE_LOG(LogTemp, Warning, TEXT("Your Navigation Mode is Walk"));
	//if (NavigationMode == EVRNavigationModes::nav_mode_fly)
	//	UE_LOG(LogTemp, Warning, TEXT("Your Navigation Mode is Fly"));
	//if (NavigationMode == EVRNavigationModes::nav_mode_none)
	//	UE_LOG(LogTemp, Warning, TEXT("Your Navigation Mode is Non"));



	if (HasContact && NavigationMode== EVRNavigationModes::nav_mode_walk) {
	   UE_LOG(LogTemp, Warning, TEXT(" ##########################################_BeginnOverlap_##########################################"));
       UE_LOG(LogTemp, Warning, TEXT(" Dist_Betw_Collision_And_ClossestPointOnSurface:                            %f "), dist_Betw_Collision_And_ClossestPointOnSurface);
	   UE_LOG(LogTemp, Warning, TEXT(" Point on the surface of collision closest to Point:                        %s "), *closestPointOnSurface.ToString());
	   UE_LOG(LogTemp, Warning, TEXT(" Camera's Position:                                                         %s "), *GetCameraComponent()->GetComponentLocation().ToString());
	  	   
	   
	   FVector Diff_Camera_and_ClosestPointOnSurface = Point - closestPointOnSurface;
	   UE_LOG(LogTemp, Warning, TEXT(" The Difference between SphereCollisionComponent and closestPointOnSurface: %s "), *Diff_Camera_and_ClosestPointOnSurface.ToString());
	   float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - dist_Betw_Collision_And_ClossestPointOnSurface;
	   UE_LOG(LogTemp, Warning, TEXT(" Inside_Distance:                                                           %f "), Inside_Distance);
	   UE_LOG(LogTemp, Warning, TEXT(" DeltaSeconds:                                                              %f "), DeltaSeconds);
	   UE_LOG(LogTemp, Warning, TEXT(" Pawn's Position:                                                           %s "), *GetRootComponent()->GetComponentLocation().ToString());
	   UE_LOG(LogTemp, Warning, TEXT(" Normalisation (Diff_Camera_and_ClosestPointOnSurface):                     %s "), *Diff_Camera_and_ClosestPointOnSurface.GetSafeNormal().ToString());
	   UE_LOG(LogTemp, Warning, TEXT(" Set The Pawn of this Point:                                                %s "), *(RootComponent->GetComponentLocation() + Diff_Camera_and_ClosestPointOnSurface.GetSafeNormal()*Inside_Distance).ToString());
	   RootComponent->AddLocalOffset(Diff_Camera_and_ClosestPointOnSurface.GetSafeNormal()*Inside_Distance, false);
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

bool AVirtualRealityPawn::CreateLineTrace(FVector Forward_OR_Right_Vector)
{
	bool isDistSmallerThenRadiusCollision = false;
	{ //LineTrace 

		FHitResult OutHit;
		FVector Start = GetCameraComponent()->GetComponentLocation();

		FVector End = ((Forward_OR_Right_Vector * 1000.f) + Start);
		FCollisionQueryParams CollisionParams;

		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				if (GEngine) {

					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));

					UE_LOG(LogTemp, Warning, TEXT(" You are hitting:       %s "), *OutHit.GetActor()->GetName());
					UE_LOG(LogTemp, Warning, TEXT(" Impact Point:          %s "), *OutHit.ImpactPoint.ToString());
					UE_LOG(LogTemp, Warning, TEXT(" Normal Point:          %s "), *OutHit.ImpactNormal.ToString());
				}


				FVector MyCamera = GetCameraComponent()->GetComponentLocation();
				FVector MyPawn = GetRootComponent()->GetComponentLocation();
				FVector MyObject = OutHit.ImpactPoint;

				float Dist_Betw_ImpactPoint_and_MyCamera = sqrt(pow((MyCamera.X - MyObject.X), 2) + pow((MyCamera.Y - MyObject.Y), 2) + pow((MyCamera.Z - MyObject.Z), 2));
				float Dist_Betw_ImpactPoint_and_MyPawn = sqrt(pow((MyPawn.X - MyObject.X), 2) + pow((MyPawn.Y - MyObject.Y), 2) + pow((MyPawn.Z - MyObject.Z), 2));

				UE_LOG(LogTemp, Warning, TEXT(" Dist_Betw_ImpactPoint_and_MyCamera:          %f "), Dist_Betw_ImpactPoint_and_MyCamera);
				UE_LOG(LogTemp, Warning, TEXT(" Dist_Betw_ImpactPoint_and_MyPawn:            %f "), Dist_Betw_ImpactPoint_and_MyPawn);

				if (Dist_Betw_ImpactPoint_and_MyCamera <= SphereCollisionComponent->GetScaledSphereRadius()) {
					isDistSmallerThenRadiusCollision = true;
				}

			}
		}
	}

	return isDistSmallerThenRadiusCollision;
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
