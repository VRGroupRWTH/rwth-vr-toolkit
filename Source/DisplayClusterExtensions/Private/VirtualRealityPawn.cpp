#include "VirtualRealityPawn.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/WorldSettings.h"
#include "Input/IDisplayClusterInputManager.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IDisplayCluster.h"

AVirtualRealityPawn::AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  MovementComponent                        = CreateDefaultSubobject<UFloatingPawnMovement>     (TEXT("MovementComponent0"));
  MovementComponent->UpdatedComponent      = RootComponent;
  
  RotatingComponent                        = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent0"));
  RotatingComponent->UpdatedComponent      = RootComponent;
  RotatingComponent->bRotationInLocalSpace = false;
  RotatingComponent->PivotTranslation      = FVector::ZeroVector;
  RotatingComponent->RotationRate          = FRotator::ZeroRotator;
  
  TranslationDirection                     = RootComponent;

  AutoPossessPlayer                        = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.
}

void                    AVirtualRealityPawn::OnForward_Implementation   (float Value)
{
  AddMovementInput(TranslationDirection->GetForwardVector(), Value);
}
void                    AVirtualRealityPawn::OnRight_Implementation     (float Value)
{
  AddMovementInput(TranslationDirection->GetRightVector  (), Value);
}
void                    AVirtualRealityPawn::OnTurnRate_Implementation  (float Rate )
{
  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
  {
    if (!RotatingComponent->UpdatedComponent || !IDisplayCluster::Get().GetGameMgr() || !IDisplayCluster::Get().GetGameMgr()->GetActiveCamera()) return;

    const FVector CameraLocation = IDisplayCluster::Get().GetGameMgr()->GetActiveCamera()->GetComponentLocation();
    RotatingComponent->PivotTranslation = RotatingComponent->UpdatedComponent->GetComponentTransform().InverseTransformPositionNoScale(CameraLocation);
    RotatingComponent->RotationRate     = FRotator(RotatingComponent->RotationRate.Pitch, Rate * BaseTurnRate, 0.0f);
  }
  else
  {
    AddControllerYawInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
  }
}
void                    AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate )
{ 
  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
  {
    // User-centered projection causes motion sickness on look up interaction hence not implemented.
  }
  else
  {
    AddControllerPitchInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
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
  
	if (!IDisplayCluster::Get().IsModuleInitialized() || !IDisplayCluster::Get().IsAvailable()) return;

  auto IsCluster = (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster);
  bUseControllerRotationYaw   = !IsCluster;
  bUseControllerRotationPitch = !IsCluster;
  bUseControllerRotationRoll  = !IsCluster;

  TArray<AActor*> SettingsActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADisplayClusterSettings::StaticClass(), SettingsActors);
  if (SettingsActors.Num() == 0) return;

  ADisplayClusterSettings* Settings = Cast<ADisplayClusterSettings>(SettingsActors[0]);
  MovementComponent->MaxSpeed       = Settings->MovementMaxSpeed    ;
  MovementComponent->Acceleration   = Settings->MovementAcceleration;
  MovementComponent->Deceleration   = Settings->MovementDeceleration;
  MovementComponent->TurningBoost   = Settings->MovementTurningBoost;
  BaseTurnRate                      = Settings->RotationSpeed       ;
  
  if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
  {
    LeftMotionControllerComponent  = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionControllerComponent"));
    LeftMotionControllerComponent->SetTrackingSource (EControllerHand::Left);
    LeftMotionControllerComponent->SetShowDeviceModel(true);

    RightMotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionControllerComponent"));
    RightMotionControllerComponent->SetTrackingSource (EControllerHand::Right);
    RightMotionControllerComponent->SetShowDeviceModel(true);

    TranslationDirection = LeftMotionControllerComponent;
  }
}
void                    AVirtualRealityPawn::Tick                       (float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

  if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster && !Flystick)
  {
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick")); // Note: Requires a scene node called flystick in the config. Make settable.
    if (Flystick) 
      TranslationDirection = Flystick;
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
    PlayerInputComponent->BindAxis                   ("MoveForward"             , this, &AVirtualRealityPawn::OnForward             );
    PlayerInputComponent->BindAxis                   ("MoveRight"               , this, &AVirtualRealityPawn::OnRight               );
    PlayerInputComponent->BindAxis                   ("TurnRate"                , this, &AVirtualRealityPawn::OnTurnRate            );
    PlayerInputComponent->BindAxis                   ("LookUpRate"              , this, &AVirtualRealityPawn::OnLookUpRate          );

    PlayerInputComponent->BindAction<FFireDelegate>  ("Fire"       , IE_Pressed , this, &AVirtualRealityPawn::OnFire      , true    );
    PlayerInputComponent->BindAction<FActionDelegate>("Action1"    , IE_Pressed , this, &AVirtualRealityPawn::OnAction    , true , 1);
    PlayerInputComponent->BindAction<FActionDelegate>("Action2"    , IE_Pressed , this, &AVirtualRealityPawn::OnAction    , true , 2);
    PlayerInputComponent->BindAction<FActionDelegate>("Action3"    , IE_Pressed , this, &AVirtualRealityPawn::OnAction    , true , 3);
    PlayerInputComponent->BindAction<FActionDelegate>("Action4"    , IE_Pressed , this, &AVirtualRealityPawn::OnAction    , true , 4);
    PlayerInputComponent->BindAction<FActionDelegate>("Action5"    , IE_Pressed , this, &AVirtualRealityPawn::OnAction    , true , 5);
    
    PlayerInputComponent->BindAction<FFireDelegate>  ("Fire"       , IE_Released, this, &AVirtualRealityPawn::OnFire      , false   );
    PlayerInputComponent->BindAction<FActionDelegate>("Action1"    , IE_Released, this, &AVirtualRealityPawn::OnAction    , false, 1);
    PlayerInputComponent->BindAction<FActionDelegate>("Action2"    , IE_Released, this, &AVirtualRealityPawn::OnAction    , false, 2);
    PlayerInputComponent->BindAction<FActionDelegate>("Action3"    , IE_Released, this, &AVirtualRealityPawn::OnAction    , false, 3);
    PlayerInputComponent->BindAction<FActionDelegate>("Action4"    , IE_Released, this, &AVirtualRealityPawn::OnAction    , false, 4);
    PlayerInputComponent->BindAction<FActionDelegate>("Action5"    , IE_Released, this, &AVirtualRealityPawn::OnAction    , false, 5);
  }
}
UPawnMovementComponent* AVirtualRealityPawn::GetMovementComponent       () const
{
  return MovementComponent;
}
