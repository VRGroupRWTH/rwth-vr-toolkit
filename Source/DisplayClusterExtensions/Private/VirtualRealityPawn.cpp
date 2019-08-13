#include "VirtualRealityPawn.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/WorldSettings.h"
#include "Input/IDisplayClusterInputManager.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IDisplayClusterConfigManager.h"
#include "IDisplayCluster.h"
#include "IXRTrackingSystem.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  AutoPossessPlayer                       = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

  Movement                                = CreateDefaultSubobject<UFloatingPawnMovement>     (TEXT("Movement"));
  Movement->UpdatedComponent              = RootComponent;
  
  RotatingMovement                        = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
  RotatingMovement->UpdatedComponent      = RootComponent;
  RotatingMovement->bRotationInLocalSpace = false;
  RotatingMovement->PivotTranslation      = FVector::ZeroVector;
  RotatingMovement->RotationRate          = FRotator::ZeroRotator;
  
  HmdLeftMotionController                    = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdLeftMotionController"));
  HmdLeftMotionController->SetupAttachment    (RootComponent);
  HmdLeftMotionController->SetTrackingSource  (EControllerHand::Left);
  HmdLeftMotionController->SetShowDeviceModel (true );
  HmdLeftMotionController->SetVisibility      (false);

  HmdRightMotionController                   = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdRightMotionController"));
  HmdRightMotionController->SetupAttachment   (RootComponent);
  HmdRightMotionController->SetTrackingSource (EControllerHand::Right);
  HmdRightMotionController->SetShowDeviceModel(true );
  HmdRightMotionController->SetVisibility     (false);
}

void                           AVirtualRealityPawn::OnForward_Implementation            (float Value)
{
  if (NavigationMode == EVRNavigationModes::NAV_MODE_FLY || IsDesktopMode() || IsHeadMountedMode()) // Check if this function triggers correctly on ROLV.
  {
    AddMovementInput(Forward->GetForwardVector(), Value);
  }
}
void                           AVirtualRealityPawn::OnRight_Implementation              (float Value)
{
  if (NavigationMode == EVRNavigationModes::NAV_MODE_FLY || IsDesktopMode() || IsHeadMountedMode())
  {
    AddMovementInput(Forward->GetRightVector(), Value);
  }
}
void                           AVirtualRealityPawn::OnTurnRate_Implementation           (float Rate )
{
  if (IsRoomMountedMode())
  {
    const FVector CameraLocation       = IDisplayCluster::Get().GetGameMgr()->GetActiveCamera()->GetComponentLocation();
    RotatingMovement->PivotTranslation = RotatingMovement->UpdatedComponent->GetComponentTransform().InverseTransformPositionNoScale(CameraLocation);
    RotatingMovement->RotationRate     = FRotator(RotatingMovement->RotationRate.Pitch, Rate * BaseTurnRate, 0.0f);
  }
  else
  {
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
  }
}
void                           AVirtualRealityPawn::OnLookUpRate_Implementation         (float Rate )
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
void                           AVirtualRealityPawn::OnFire_Implementation               (bool Pressed)
{ 

}
void                           AVirtualRealityPawn::OnAction_Implementation             (bool Pressed, int32 Index)
{ 

}
                               
bool                           AVirtualRealityPawn::IsDesktopMode                       ()
{
  return !IsRoomMountedMode() && !IsHeadMountedMode();
}
bool                           AVirtualRealityPawn::IsRoomMountedMode                   ()
{
  return IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster;
}
bool                           AVirtualRealityPawn::IsHeadMountedMode                   ()
{
  return GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
}

FString                        AVirtualRealityPawn::GetNodeName                         ()
{
  return IsRoomMountedMode() ? IDisplayCluster::Get().GetClusterMgr()->GetNodeId() : FString(TEXT("Localhost"));
}
float                          AVirtualRealityPawn::GetEyeDistance()
{
  return IDisplayCluster::Get().GetConfigMgr()->GetConfigStereo().EyeDist;
}

float                          AVirtualRealityPawn::GetBaseTurnRate                     () const
{
  return BaseTurnRate;
}
UFloatingPawnMovement*         AVirtualRealityPawn::GetFloatingPawnMovement             ()
{
  return Movement;
}
URotatingMovementComponent*    AVirtualRealityPawn::GetRotatingMovementComponent        ()
{
  return RotatingMovement;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetFlystickComponent                ()
{
  return Flystick;
}
UMotionControllerComponent*    AVirtualRealityPawn::GetHmdLeftMotionControllerComponent ()
{
  return HmdLeftMotionController;
}
UMotionControllerComponent*    AVirtualRealityPawn::GetHmdRightMotionControllerComponent()
{
  return HmdRightMotionController;
}

USceneComponent*               AVirtualRealityPawn::GetForwardComponent                 ()
{
  return Forward;
}
USceneComponent*               AVirtualRealityPawn::GetLeftHandComponent                ()
{
  return LeftHand;
}
USceneComponent*               AVirtualRealityPawn::GetRightHandComponent               ()
{
  return RightHand;
}

UDisplayClusterSceneComponent* AVirtualRealityPawn::GetClusterComponent                 (const FString& name)
{
  return IDisplayCluster::Get().GetGameMgr()->GetNodeById(name);
}

void                           AVirtualRealityPawn::BeginPlay                           ()
{
  Super::BeginPlay();
  
  bUseControllerRotationYaw   = true;
  bUseControllerRotationPitch = true;
  bUseControllerRotationRoll  = true;
  
  // Display cluster settings apply to all setups (PC, HMD, CAVE/ROLV) despite the unfortunate name due to being an UE4 internal.
  TArray<AActor*> SettingsActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADisplayClusterSettings::StaticClass(), SettingsActors);
  if (SettingsActors.Num() > 0)
  {  
    ADisplayClusterSettings* Settings = Cast<ADisplayClusterSettings>(SettingsActors[0]);
    Movement->MaxSpeed     = Settings->MovementMaxSpeed    ;
    Movement->Acceleration = Settings->MovementAcceleration;
    Movement->Deceleration = Settings->MovementDeceleration;
    Movement->TurningBoost = Settings->MovementTurningBoost;
    BaseTurnRate           = Settings->RotationSpeed       ;
  }

  if      (IsRoomMountedMode())
  {
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("TurnRate"  )), EKeys::MouseX));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("LookUpRate")), EKeys::MouseY));

    // Requires a scene node called flystick in the config.
    Flystick  = GetClusterComponent(TEXT("flystick"));

    Forward   = Flystick;
    LeftHand  = Flystick;
    RightHand = Flystick;

    RootComponent->SetWorldLocation(FVector(0, 2, 0), false, nullptr, ETeleportType::None);
  }
  else if (IsHeadMountedMode())
  {
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("TurnRate"  )), EKeys::MouseX));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("LookUpRate")), EKeys::MouseY));

    HmdLeftMotionController ->SetVisibility(true);
    HmdRightMotionController->SetVisibility(true);
    
    Forward   = HmdLeftMotionController ;
    LeftHand  = HmdLeftMotionController ;
    RightHand = HmdRightMotionController;
  }
  else
  {
    Forward   = RootComponent;
    LeftHand  = RootComponent;
    RightHand = RootComponent;
  }
}
void                           AVirtualRealityPawn::Tick                                (float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
  
  // Flystick might not be available at start, hence is checked every frame.
  if (IsRoomMountedMode() && !Flystick)
  { 
    // Requires a scene node called flystick in the config.
    Flystick  = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick"));

    Forward   = Flystick;
    LeftHand  = Flystick;
    RightHand = Flystick;
  }
}
void                           AVirtualRealityPawn::BeginDestroy                        ()
{
  Super::BeginDestroy();
}
                              
void                           AVirtualRealityPawn::SetupPlayerInputComponent           (UInputComponent* PlayerInputComponent)
{
  check(PlayerInputComponent);
  Super::SetupPlayerInputComponent(PlayerInputComponent);
  if (PlayerInputComponent)
  {
    PlayerInputComponent->BindAxis                   ("MoveForward"    , this, &AVirtualRealityPawn::OnForward   );
    PlayerInputComponent->BindAxis                   ("MoveRight"      , this, &AVirtualRealityPawn::OnRight     );
    PlayerInputComponent->BindAxis                   ("TurnRate"       , this, &AVirtualRealityPawn::OnTurnRate  );
    PlayerInputComponent->BindAxis                   ("LookUpRate"     , this, &AVirtualRealityPawn::OnLookUpRate);

    // The button names are based on the definitions in aixcave_422.cfg.
    PlayerInputComponent->BindAction<FFireDelegate>  ("nDisplayButton0", IE_Pressed , this, &AVirtualRealityPawn::OnFire  , true    );
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton1", IE_Pressed , this, &AVirtualRealityPawn::OnAction, true , 1);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton2", IE_Pressed , this, &AVirtualRealityPawn::OnAction, true , 2);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton3", IE_Pressed , this, &AVirtualRealityPawn::OnAction, true , 3);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton4", IE_Pressed , this, &AVirtualRealityPawn::OnAction, true , 4);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton5", IE_Pressed , this, &AVirtualRealityPawn::OnAction, true , 5);

    PlayerInputComponent->BindAction<FFireDelegate>  ("nDisplayButton0", IE_Released, this, &AVirtualRealityPawn::OnFire  , false   );
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton1", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 1);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton2", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 2);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton3", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 3);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton4", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 4);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplayButton5", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 5);
  }
}
UPawnMovementComponent*        AVirtualRealityPawn::GetMovementComponent                () const
{
  return Movement;
}
