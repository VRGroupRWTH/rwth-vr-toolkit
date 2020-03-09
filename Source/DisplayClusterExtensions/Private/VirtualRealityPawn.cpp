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
	
	bool isDistSmallerThenRadiusCollision_ForwardVector = CreateLineTrace(GetCameraComponent()->GetForwardVector(), GetCameraComponent()->GetComponentLocation(), false);

	bool isDist_RightHand_ForwardVector = CreateLineTrace(RightHand->GetForwardVector(), RightHand->GetComponentLocation(), false);
	bool isDist_RightHand_BackVector = CreateLineTrace(-RightHand->GetForwardVector(), RightHand->GetComponentLocation(), false);

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
		if (isDistSmallerThenRadiusCollision_ForwardVector|| (isDist_RightHand_ForwardVector  && Value > 0.0f )|| (isDist_RightHand_BackVector && Value < 0.0f)) {
			Value = 0;
		}
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}

	{//Gravity

		//FVector ImpactPoint_Down12 = CreateLineTrace_Return_OutHit_ImpactPoint(FVector(GetCameraComponent()->GetForwardVector().X, GetCameraComponent()->GetForwardVector().Y,0.f), GetCameraComponent()->GetComponentLocation(), true);
		//UE_LOG(LogTemp, Warning, TEXT(" *GetCameraComponent()->GetForwardVector().ToString():                            %s "), *GetCameraComponent()->GetForwardVector().ToString());
		FVector ImpactPoint_Down = CreateLineTrace_Return_OutHit_ImpactPoint(FVector(0, 0, -1), GetCameraComponent()->GetComponentLocation(), false);
		float Dist_Betw_Camera_A_Ground_Z = abs(ImpactPoint_Down.Z - GetCameraComponent()->GetComponentLocation().Z);
		float Dist_Betw_Camera_A_Pawn_Z = abs(RootComponent->GetComponentLocation().Z - GetCameraComponent()->GetComponentLocation().Z);
		float Differnce_Distance = abs(Dist_Betw_Camera_A_Ground_Z - Dist_Betw_Camera_A_Pawn_Z);

		{//Nicht den Hocker hochgehen.
			float Stufenhoehe_cm = 30.f;
			FVector Start_From_Knee = FVector(GetCameraComponent()->GetComponentLocation().X, GetCameraComponent()->GetComponentLocation().Y, GetCameraComponent()->GetComponentLocation().Z - (Dist_Betw_Camera_A_Ground_Z - Stufenhoehe_cm));
			//Frage: Warum ist ImpactPoint_Belly_Forward nach hinten zeigt?????????????????????????????????????? : FVector(1.f, 0.f, 0.f) ????
			FVector Vector_Forward = FVector(GetCameraComponent()->GetForwardVector().X, GetCameraComponent()->GetForwardVector().Y, 0.f);
			FVector Vector_Right = FVector(GetCameraComponent()->GetRightVector().X, GetCameraComponent()->GetRightVector().Y, 0.f);

			FVector ImpactPoint_Belly_Forward = CreateLineTrace_Return_OutHit_ImpactPoint(Vector_Forward, Start_From_Knee, true);
			FVector ImpactPoint_Belly_Backward = CreateLineTrace_Return_OutHit_ImpactPoint(-Vector_Forward, Start_From_Knee, true);
			FVector ImpactPoint_Belly_Right = CreateLineTrace_Return_OutHit_ImpactPoint(Vector_Right, Start_From_Knee, true);
			FVector ImpactPoint_Belly_Left = CreateLineTrace_Return_OutHit_ImpactPoint(-Vector_Right, Start_From_Knee, true);

			if (FVector::Distance(ImpactPoint_Belly_Forward, Start_From_Knee) < SphereCollisionComponent->GetScaledSphereRadius() && !OnForwardClicked) {
				
				FVector Diff_ImpactPoint_Belly_Forward_And_Start_From_Knee = ImpactPoint_Belly_Forward - Start_From_Knee;
				float Inside_Distance = SphereCollisionComponent->GetScaledSphereRadius() - FVector::Distance(ImpactPoint_Belly_Forward, Start_From_Knee);
				RootComponent->AddLocalOffset(Diff_ImpactPoint_Belly_Forward_And_Start_From_Knee.GetSafeNormal()*Inside_Distance, true);
				UE_LOG(LogTemp, Warning, TEXT(" FVector::Distance(ImpactPoint_Belly_Backward, Start_From_Knee):                            %f "), FVector::Distance(ImpactPoint_Belly_Forward, Start_From_Knee));
				UE_LOG(LogTemp, Warning, TEXT(" Inside_Distance:                            %f "), Inside_Distance);
				UE_LOG(LogTemp, Warning, TEXT(" *Diff_ImpactPoint_Belly_Forward_And_Start_From_Knee.GetSafeNormal().ToString():                            %s "), *Diff_ImpactPoint_Belly_Forward_And_Start_From_Knee.GetSafeNormal().ToString());
			
			}
			else if (FVector::Distance(ImpactPoint_Belly_Forward, Start_From_Knee) < 40.f && OnForwardClicked) {
				AddMovementInput(RightHand->GetForwardVector(), 0.f);
			}

		}


		//if you not have ImpactPoint, then you are falling.
		if (ImpactPoint_Down == FVector(0.f, 0.f, 0.f)) {
			static float Gravity_Speed = 0.0;
			Gravity_Speed += 0.05;
			UE_LOG(LogTemp, Warning, TEXT(" Gravity_Speed:                            %f "), Gravity_Speed);
			FVector GravityAcc = FVector(0.f, 0.f, -1.f);
			const FVector LocalMove = FVector(0.f, 0.f, 0.f) + GravityAcc;
			RootComponent->AddLocalOffset(LocalMove*Gravity_Speed, true);
		}
		//Treppe hochgehen/runtergehen.
		else {
            if (Dist_Betw_Camera_A_Ground_Z < Dist_Betw_Camera_A_Pawn_Z) {
				const FVector Local_UpMove{ 0.f, 0.f, +Differnce_Distance };
				RootComponent->AddLocalOffset(Local_UpMove, true);
			}
			else if (Dist_Betw_Camera_A_Ground_Z > Dist_Betw_Camera_A_Pawn_Z) {
				const FVector Local_UpMove{ 0.f, 0.f, -Differnce_Distance };
				RootComponent->AddLocalOffset(Local_UpMove, true);
			}
		}
	}
	/*
	{//Gravity
	  FVector GravityAcc = FVector(0.f, 0.f, -1.f) * 5.f;
	  const FVector LocalMove = FVector(100 * DeltaSeconds, 0.f, 0.f) + GravityAcc;
	  RootComponent->AddLocalOffset(LocalMove, true);
	}
	*/

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
       //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap Begin"));
       //UE_LOG(LogTemp, Warning, TEXT("Overlap Begin"));
       HasContact = true;
	   Point = SphereCollisionComponent->GetComponentLocation(); //World 3D vector
	   dist_Betw_Collision_And_ClossestPointOnSurface = OtherComp->GetDistanceToCollision(Point, closestPointOnSurface);//Gibt die Entfernung zur nächstgelegenen Körperinstanzoberfläche zurück.
	
	 }
	
}

void AVirtualRealityPawn::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{

	if (OtherActor && (OtherActor != this) && OtherComp) {
       //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap End"));
	   //UE_LOG(LogTemp, Warning, TEXT("Overlap End"));
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


}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Flystick might not be available at start, hence is checked every frame.
	InitComponentReferences();

	

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

bool AVirtualRealityPawn::CreateLineTrace(FVector Forward_Right_OR_Up_Vector, const FVector MyObjekt_ComponentLocation, bool Visibility)
{
	bool isDistSmallerThenRadiusCollision = false;
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
			
				FVector MyObjekt_Vector = Start;
				FVector MyImpactPoint_Vector = OutHit.ImpactPoint;

				float Dist_Betw_ImpactPoint_and_MyObjekt = FVector::Distance(MyObjekt_Vector, MyImpactPoint_Vector);

				if (Dist_Betw_ImpactPoint_and_MyObjekt <= SphereCollisionComponent->GetScaledSphereRadius()) {
					isDistSmallerThenRadiusCollision = true;
				}

			}
		}
	}

	return isDistSmallerThenRadiusCollision;
}

FVector AVirtualRealityPawn::CreateLineTrace_Return_OutHit_ImpactPoint(FVector Forward_Right_OR_Up_Vector, const FVector MyObjekt_ComponentLocation, bool Visibility)
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
