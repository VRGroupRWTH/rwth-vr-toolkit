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
}

void AVirtualRealityPawn::OnForward_Implementation(float Value)
{
	// Check if this function triggers correctly on ROLV.
	if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || IsDesktopMode() || IsHeadMountedMode()))
	{
		AddMovementInput(RightHand->GetForwardVector(), Value);
	}
}

void AVirtualRealityPawn::OnRight_Implementation(float Value)
{
	if (RightHand && (NavigationMode == EVRNavigationModes::nav_mode_fly || IsDesktopMode() || IsHeadMountedMode()))
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
	if (IsRoomMountedMode())
	{
		// User-centered projection causes simulation sickness on look up interaction hence not implemented.
	}
	else
	{
		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	}
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

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetClusterComponent(const FString& Name)
{
	return IDisplayCluster::Get().GetGameMgr()->GetNodeById(Name);
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

	if (IsRoomMountedMode())
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		InitRoomMountedComponentReferences();
	}
	else if (IsHeadMountedMode())
	{
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
		UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));

		HmdLeftMotionController->SetVisibility(true);
		HmdRightMotionController->SetVisibility(true);

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
}

void AVirtualRealityPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	if (!IsRoomMountedMode()) return;
	if (!TrackingOrigin) TrackingOrigin = GetClusterComponent("cave_origin");
        if (!TrackingOrigin) TrackingOrigin = GetClusterComponent("rolv_origin");
	if (!CaveCenter) CaveCenter = GetClusterComponent("cave_center");
	if (!ShutterGlasses)
	{
		ShutterGlasses = GetClusterComponent("shutter_glasses");
                Head->AttachToComponent(ShutterGlasses, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (!Flystick)
	{
		Flystick = GetClusterComponent("flystick");

		LeftHand->AttachToComponent(Flystick, FAttachmentTransformRules::KeepRelativeTransform);
		RightHand->AttachToComponent(Flystick, FAttachmentTransformRules::KeepRelativeTransform);
	}
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

