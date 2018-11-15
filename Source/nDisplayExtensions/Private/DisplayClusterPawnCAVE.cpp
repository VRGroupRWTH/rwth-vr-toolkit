#include "DisplayClusterPawnCAVE.h"

#include "Game/IDisplayClusterGameManager.h"
#include "IDisplayCluster.h"

void ADisplayClusterPawnCAVE::BeginPlay()
{
  Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById("flystick");
}
void ADisplayClusterPawnCAVE::MoveForward(float value)
{
  if (!Flystick)
    Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById("flystick");
  if (!Flystick || value == 0.0f)
    return;
  AddMovementInput(Flystick->GetForwardVector(), value);
}
