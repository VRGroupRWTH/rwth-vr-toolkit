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
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn constructor started"));
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn constructor finished"));
}

void                    AVirtualRealityPawn::OnForward_Implementation   (float Value)
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnForward_Implementation started"));
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnForward_Implementation finished"));
}
void                    AVirtualRealityPawn::OnRight_Implementation(float Value)
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnRight_Implementation started"));
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnRight_Implementation finished"));
}
void                    AVirtualRealityPawn::OnTurnRate_Implementation  (float Rate )
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnTurnRate_Implementation started"));
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnTurnRate_Implementation finished"));
}
void                    AVirtualRealityPawn::OnLookUpRate_Implementation(float Rate )
{ 
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnLookUpRate_Implementation started"));
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn OnLookUpRate_Implementation finished"));
}
void                    AVirtualRealityPawn::OnFire_Implementation      (bool Pressed)
{ 

}
void                    AVirtualRealityPawn::OnAction_Implementation    (bool Pressed, int32 Index)
{ 

}

void                    AVirtualRealityPawn::BeginPlay                  ()
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn BeginPlay started"));
  Super::BeginPlay();
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn BeginPlay finished"));
}
void                    AVirtualRealityPawn::Tick                       (float DeltaSeconds)
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn Tick started"));
  Super::Tick(DeltaSeconds);
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn Tick finished"));
}
void                    AVirtualRealityPawn::BeginDestroy               ()
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn BeginDestroy started"));
  Super::BeginDestroy();
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn BeginDestroy finished"));
}

void                    AVirtualRealityPawn::SetupPlayerInputComponent  (UInputComponent* PlayerInputComponent)
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn SetupPlayerInputComponent started"));
  Super::SetupPlayerInputComponent(PlayerInputComponent);
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityPawn SetupPlayerInputComponent finished"));
}
UPawnMovementComponent* AVirtualRealityPawn::GetMovementComponent       () const
{
  return Movement;
}
