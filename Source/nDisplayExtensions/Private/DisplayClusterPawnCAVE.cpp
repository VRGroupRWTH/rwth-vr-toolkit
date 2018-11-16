#include "DisplayClusterPawnCAVE.h"

#include "Game/IDisplayClusterGameManager.h"
#include "IDisplayCluster.h"

void ADisplayClusterPawnCAVE::BeginPlay()
{
  Super::BeginPlay();
  Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById("flystick");
}
void ADisplayClusterPawnCAVE::MoveForward(float Value)
{
  if (!Flystick)
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById("flystick");
  Flystick ? AddMovementInput(Flystick->GetForwardVector(), Value) : Super::MoveForward(Value);
}
