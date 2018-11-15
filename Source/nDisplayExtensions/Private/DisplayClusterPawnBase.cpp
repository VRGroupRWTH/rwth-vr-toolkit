#include "DisplayClusterPawnBase.h"

#include "Engine/World.h"
#include "Game/IDisplayClusterGameManager.h"
#include "GameFramework/WorldSettings.h"
#include "DisplayClusterSceneComponentSyncParent.h"
#include "DisplayClusterSettings.h"
#include "DisplayClusterGameMode.h"
#include "IDisplayCluster.h"
#include "Kismet/GameplayStatics.h"

ADisplayClusterPawnBase::ADisplayClusterPawnBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MovementComponent                         = CreateDefaultSubobject<UFloatingPawnMovement>     (TEXT("MovementComponent0"));
	MovementComponent->UpdatedComponent       = RootComponent;

	RotatingComponent                         = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent0"));
	RotatingComponent->UpdatedComponent       = RootComponent;
	RotatingComponent->bRotationInLocalSpace  = true;
	RotatingComponent->PivotTranslation       = FVector::ZeroVector;
	RotatingComponent->RotationRate           = FRotator::ZeroRotator;

	RotatingComponent2                        = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent1"));
	RotatingComponent2->UpdatedComponent      = RootComponent;
	RotatingComponent2->bRotationInLocalSpace = false;
	RotatingComponent2->PivotTranslation      = FVector::ZeroVector;
	RotatingComponent2->RotationRate          = FRotator::ZeroRotator;

	BaseTurnRate   = 45.f;
	BaseLookUpRate = 45.f;
}

void ADisplayClusterPawnBase::MoveForward (float Value)
{
  if (Value == 0.f) return;
	AddMovementInput((TranslationDirection ? TranslationDirection : RootComponent)->GetForwardVector(), Value);
}
void ADisplayClusterPawnBase::MoveRight   (float Value)
{
  if (Value == 0.f) return;
  AddMovementInput((TranslationDirection ? TranslationDirection : RootComponent)->GetRightVector  (), Value);
}
void ADisplayClusterPawnBase::MoveUp      (float Value)
{
  if (Value == 0.f) return;
  AddMovementInput((TranslationDirection ? TranslationDirection : RootComponent)->GetUpVector     (), Value);
}
void ADisplayClusterPawnBase::TurnAtRate  (float Rate )
{
	if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
	{
    if (!RotatingComponent->UpdatedComponent) return;
		auto Manager = IDisplayCluster::Get().GetGameMgr();
    if (!Manager) return;
    auto Camera = Manager->GetActiveCamera();
    if (!Camera ) return;

		const FTransform TransformToRotate  = RotatingComponent->UpdatedComponent->GetComponentTransform();
		const FVector    RotateAroundPivot  = TransformToRotate.InverseTransformPositionNoScale(Camera->GetComponentLocation());
		RotatingComponent->PivotTranslation = RotateAroundPivot;
		RotatingComponent->RotationRate     = FRotator(RotatingComponent->RotationRate.Pitch, Rate * BaseTurnRate, 0.f);
	}
  else if (Rate != 0.f)
		AddControllerYawInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}
void ADisplayClusterPawnBase::TurnAtRate2 (float Rate )
{
	if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
	{
    if (!RotatingComponent2->UpdatedComponent) return;
    auto Manager = IDisplayCluster::Get().GetGameMgr();
    if (!Manager) return;
    auto Camera  = Manager->GetActiveCamera();
    if (!Camera ) return;

		const FTransform TransformToRotate   = RotatingComponent2->UpdatedComponent->GetComponentTransform();
		const FVector    RotateAroundPivot   = TransformToRotate.InverseTransformPositionNoScale(Camera->GetComponentLocation());
		RotatingComponent2->PivotTranslation = RotateAroundPivot;
		RotatingComponent2->RotationRate     = FRotator(RotatingComponent2->RotationRate.Pitch, Rate * BaseTurnRate, 0.f);
	}
	else if (Rate != 0.f)
		AddControllerYawInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}
void ADisplayClusterPawnBase::LookUpAtRate(float Rate )
{
	if (IDisplayCluster::Get().GetOperationMode() == EDisplayClusterOperationMode::Cluster)
	{
		//@note: usually CAVE-like systems don't use roll and pitch rotation since it can cause dizziness.
#if 0
		//@todo: rotate around active camera
    auto Manager = IDisplayCluster::Get().GetGameMgr();
    if (!Manager) return;
    auto Camera = Manager->GetActiveCamera();
    if (!Camera ) return;

		RotatingComponent->bRotationInLocalSpace = true;
		RotatingComponent->PivotTranslation      = FVector::ZeroVector;
		
#endif
	}
	else if (Rate != 0.f)
	{
		AddControllerPitchInput(BaseTurnRate * Rate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
	}
}

void                    ADisplayClusterPawnBase::BeginPlay           ()
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
  MovementComponent->MaxSpeed     = Settings->MovementMaxSpeed;
  MovementComponent->Acceleration = Settings->MovementAcceleration;
  MovementComponent->Deceleration = Settings->MovementDeceleration;
  MovementComponent->TurningBoost = Settings->MovementTurningBoost;
  BaseTurnRate                    = Settings->RotationSpeed;
  BaseLookUpRate                  = Settings->RotationSpeed;
}
void                    ADisplayClusterPawnBase::Tick                (float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Mult = GetWorld()->GetWorldSettings()->WorldToMeters / 100.f;
	SetActorScale3D(FVector(Mult, Mult, Mult));
}
UPawnMovementComponent* ADisplayClusterPawnBase::GetMovementComponent() const
{
  return MovementComponent;
}

void ADisplayClusterPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAxis("MoveForward", this, &ADisplayClusterPawnBase::MoveForward );
		PlayerInputComponent->BindAxis("MoveRight"  , this, &ADisplayClusterPawnBase::MoveRight   );
		PlayerInputComponent->BindAxis("MoveUp"     , this, &ADisplayClusterPawnBase::MoveUp      );
		PlayerInputComponent->BindAxis("TurnRate"   , this, &ADisplayClusterPawnBase::TurnAtRate2 );
		PlayerInputComponent->BindAxis("LookUpRate" , this, &ADisplayClusterPawnBase::LookUpAtRate);
	}
}
