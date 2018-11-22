#include "DisplayClusterPawnCAVE.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/WorldSettings.h"
#include "Input/IDisplayClusterInputManager.h"
#include "Kismet/GameplayStatics.h"
#include "DisplayClusterSettings.h"
#include "IDisplayCluster.h"

ADisplayClusterPawnCAVE::ADisplayClusterPawnCAVE(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MovementComponent                        = CreateDefaultSubobject<UFloatingPawnMovement>     (TEXT("MovementComponent0"));
	MovementComponent->UpdatedComponent      = RootComponent;

	RotatingComponent                        = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent0"));
	RotatingComponent->UpdatedComponent      = RootComponent;
	RotatingComponent->bRotationInLocalSpace = false;
	RotatingComponent->PivotTranslation      = FVector::ZeroVector;
	RotatingComponent->RotationRate          = FRotator::ZeroRotator;
}

void                    ADisplayClusterPawnCAVE::OnForward_Implementation   (float Value) 
{
  if (Flystick)
  {
    if (IDisplayCluster::Get().GetClusterMgr()->IsMaster()) 
      AddMovementInput(Flystick->GetForwardVector(), Value);
  }
  else
  {
    AddMovementInput((TranslationDirection ? TranslationDirection : RootComponent)->GetForwardVector(), Value);
  }
}
void                    ADisplayClusterPawnCAVE::OnRight_Implementation     (float Value)
{ 
  if (Flystick)
  {
    if (IDisplayCluster::Get().GetClusterMgr()->IsMaster())
      AddMovementInput(Flystick->GetRightVector(), Value);
  }
  else
  {
    AddMovementInput((TranslationDirection ? TranslationDirection : RootComponent)->GetRightVector(), Value);
  }
}
void                    ADisplayClusterPawnCAVE::OnTurnRate_Implementation  (float Rate )
{
  if (Flystick)
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
void                    ADisplayClusterPawnCAVE::OnLookUpRate_Implementation(float Rate )
{ 
  if (Flystick)
  {
    // User-centered projection causes motion sickness on look up interaction hence not implemented.
  }
  else
  {
    AddControllerPitchInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
  }
}
void                    ADisplayClusterPawnCAVE::OnFire_Implementation      ()
{ 

}
void                    ADisplayClusterPawnCAVE::OnAction_Implementation    (int32 Index)
{ 

}

void                    ADisplayClusterPawnCAVE::BeginPlay                  ()
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
}
void                    ADisplayClusterPawnCAVE::Tick                       (float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

  // Is this really necessary?
	const float Mult = GetWorld()->GetWorldSettings()->WorldToMeters / 100.f;
	SetActorScale3D(FVector(Mult, Mult, Mult));

  if (!Flystick) 
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(TEXT("flystick")); // There MUST be an scene node called flystick in the config.
}
void                    ADisplayClusterPawnCAVE::BeginDestroy               ()
{
  Super::BeginDestroy();
}

void                    ADisplayClusterPawnCAVE::SetupPlayerInputComponent  (UInputComponent* PlayerInputComponent)
{
  check(PlayerInputComponent);
  Super::SetupPlayerInputComponent(PlayerInputComponent);
  if (PlayerInputComponent)
  {
    PlayerInputComponent->BindAxis                   ("MoveForward"             , this, &ADisplayClusterPawnCAVE::OnForward   );
    PlayerInputComponent->BindAxis                   ("MoveRight"               , this, &ADisplayClusterPawnCAVE::OnRight     );
    PlayerInputComponent->BindAxis                   ("TurnRate"                , this, &ADisplayClusterPawnCAVE::OnTurnRate  );
    PlayerInputComponent->BindAxis                   ("LookUpRate"              , this, &ADisplayClusterPawnCAVE::OnLookUpRate);
    PlayerInputComponent->BindAction                 ("Fire"       , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnFire      );
    PlayerInputComponent->BindAction<FButtonDelegate>("Action1"    , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnAction , 1);
    PlayerInputComponent->BindAction<FButtonDelegate>("Action2"    , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnAction , 2);
    PlayerInputComponent->BindAction<FButtonDelegate>("Action3"    , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnAction , 3);
    PlayerInputComponent->BindAction<FButtonDelegate>("Action4"    , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnAction , 4);
    PlayerInputComponent->BindAction<FButtonDelegate>("Action5"    , IE_Pressed , this, &ADisplayClusterPawnCAVE::OnAction , 5);
  }
}
UPawnMovementComponent* ADisplayClusterPawnCAVE::GetMovementComponent       () const
{
  return MovementComponent;
}
