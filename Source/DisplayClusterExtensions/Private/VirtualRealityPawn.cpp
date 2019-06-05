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
#include "IDisplayCluster.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

  Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
  Movement->UpdatedComponent = RootComponent;

  RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
  RotatingMovement->UpdatedComponent = RootComponent;
  RotatingMovement->bRotationInLocalSpace = false;
  RotatingMovement->PivotTranslation = FVector::ZeroVector;
  RotatingMovement->RotationRate = FRotator::ZeroRotator;

  LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
  LeftMotionController->SetupAttachment(RootComponent);
  LeftMotionController->SetTrackingSource(EControllerHand::Left);
  LeftMotionController->SetShowDeviceModel(true);
  LeftMotionController->SetVisibility(false);

  RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
  RightMotionController->SetupAttachment(RootComponent);
  RightMotionController->SetTrackingSource(EControllerHand::Right);
  RightMotionController->SetShowDeviceModel(true);
  RightMotionController->SetVisibility(false);
}

void                    AVirtualRealityPawn::OnForward_Implementation   (float Value)
{
  if (NavigationMode == EVRNavigationModes::NAV_MODE_FLY || IDisplayCluster::Get().GetClusterMgr()->IsStandalone())
  {
    AddMovementInput(Forward->GetForwardVector(), Value);
  }
}
void                    AVirtualRealityPawn::OnRight_Implementation(float Value)
{
  if (NavigationMode == EVRNavigationModes::NAV_MODE_FLY || IDisplayCluster::Get().GetClusterMgr()->IsStandalone())
  {
    AddMovementInput(Forward->GetRightVector(), Value);
  }
}
void                    AVirtualRealityPawn::OnTurnRate_Implementation  (float Rate )
{
  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
  {
    const FVector CameraLocation = IDisplayCluster::Get().GetGameMgr()->GetActiveCamera()->GetComponentLocation();
    RotatingMovement->PivotTranslation = RotatingMovement->UpdatedComponent->GetComponentTransform().InverseTransformPositionNoScale(CameraLocation);
    RotatingMovement->RotationRate = FRotator(RotatingMovement->RotationRate.Pitch, Rate * BaseTurnRate, 0.0f);
  }
  else
  {
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
  }
}
void                    AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate )
{ 
  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
  {
    // User-centered projection causes simulation sickness on look up interaction hence not implemented.
  }
  else
  {
    AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
  }
}
void                    AVirtualRealityPawn::OnFire_Implementation      (bool Pressed)
{ 

}
void                    AVirtualRealityPawn::OnAction_Implementation    (bool Pressed, int32 Index)
{ 

}

void                    AVirtualRealityPawn::BeginPlay                  ()
{
  Super::BeginPlay();
  bUseControllerRotationYaw = true;
  bUseControllerRotationPitch = true;
  bUseControllerRotationRoll = true;

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

  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
  {
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("TurnRate")), EKeys::MouseX));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("LookUpRate")), EKeys::MouseY));

    // Requires a scene node called flystick in the config.
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick"));

    Forward = Flystick;
    LeftHand = Flystick;
    RightHand = Flystick;
  }
  else if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected())
  {
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("TurnRate")), EKeys::MouseX));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping(FName(TEXT("LookUpRate")), EKeys::MouseY));

    LeftMotionController->SetVisibility(true);
    RightMotionController->SetVisibility(true);

    Forward = LeftMotionController;
    LeftHand = LeftMotionController;
    RightHand = RightMotionController;
  }
  else
  {
    Forward = RootComponent;
    LeftHand = RootComponent;
    RightHand = RootComponent;
  }
}
void                    AVirtualRealityPawn::Tick                       (float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  //Flystick might not be available at start, hence is checked every frame.
  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster && !Flystick)
  {
    // Requires a scene node called flystick in the config.
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick"));
  
    Forward = Flystick;
    LeftHand = Flystick;
    RightHand = Flystick;
  }
}
void                    AVirtualRealityPawn::BeginDestroy               ()
{
  Super::BeginDestroy();
}

void                    AVirtualRealityPawn::SetupPlayerInputComponent  (UInputComponent* PlayerInputComponent)
{
 check(PlayerInputComponent);
  Super::SetupPlayerInputComponent(PlayerInputComponent);
  if (PlayerInputComponent)
  {

    // needs potentially [input_setup] id=dtrack_axis ch=0 bind="nDisplay Analog 0"
    //PlayerInputComponent->BindAxis("nDisplay Analog 0", this, &AVirtualRealityPawn::OnForward);
    //PlayerInputComponent->BindAxis("nDisplay Analog 1", this, &AVirtualRealityPawn::OnRight);
    //PlayerInputComponent->BindAxis("TurnRate", this, &AVirtualRealityPawn::OnTurnRate);
    //PlayerInputComponent->BindAxis("LookUpRate", this, &AVirtualRealityPawn::OnLookUpRate);
    //
    //need [input_setup] id=dtrack_buttons ch=1 bind="nDisplay Button 1"
    //     [input_setup] id=dtrack_buttons ch=2 bind="nDisplay Button 2"
    //PlayerInputComponent->BindAction<FFireDelegate>("nDisplay Button 1", IE_Pressed, this, &AVirtualRealityPawn::OnFire, true);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 1", IE_Pressed, this, &AVirtualRealityPawn::OnAction, true, 1);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 2", IE_Pressed, this, &AVirtualRealityPawn::OnAction, true, 2);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 3", IE_Pressed, this, &AVirtualRealityPawn::OnAction, true, 3);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 4", IE_Pressed, this, &AVirtualRealityPawn::OnAction, true, 4);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 5", IE_Pressed, this, &AVirtualRealityPawn::OnAction, true, 5);
    //
    //PlayerInputComponent->BindAction<FFireDelegate>("Fire", IE_Released, this, &AVirtualRealityPawn::OnFire, false);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 1", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 1);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 2", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 2);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 3", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 3);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 4", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 4);
    PlayerInputComponent->BindAction<FActionDelegate>("nDisplay Button 5", IE_Released, this, &AVirtualRealityPawn::OnAction, false, 5);
  }
}
UPawnMovementComponent* AVirtualRealityPawn::GetMovementComponent       () const
{
  return Movement;
}
