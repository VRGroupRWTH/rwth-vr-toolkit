#include "DisplayClusterPawnCAVE.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "Game/IDisplayClusterGameManager.h"
#include "Input/IDisplayClusterInputManager.h"
#include "IDisplayCluster.h"

void ADisplayClusterPawnCAVE::OnAxisEvent_Implementation   (FVector2D Value       )
{
  if (!IDisplayCluster::Get().GetClusterMgr()->IsMaster()) return;
  AddMovementInput(Flystick->GetRightVector  (), Value[0]);
  AddMovementInput(Flystick->GetForwardVector(), Value[1]);
}
void ADisplayClusterPawnCAVE::OnTriggerEvent_Implementation(bool      Pressed     ) 
{

}
void ADisplayClusterPawnCAVE::OnButtonEvent_Implementation (bool      Pressed     , int32 Index) 
{

}

void ADisplayClusterPawnCAVE::Tick                         (float     DeltaSeconds)
{
  // Due to these declarations, this class is bound to aixcave.cfg.
  static const auto flystick_name = FString(TEXT("flystick"      ));
  static const auto axis_name     = FString(TEXT("dtrack_axis"   ));
  static const auto buttons_name  = FString(TEXT("dtrack_buttons"));

	Super::Tick(DeltaSeconds);

  if (!Flystick) Flystick = IDisplayCluster::Get().GetGameMgr()->GetNodeById(flystick_name);
  if (!Flystick) return;

  FVector2D Axes;
  IDisplayCluster::Get().GetInputMgr()->GetAxis(axis_name, 0, Axes[0]);
  IDisplayCluster::Get().GetInputMgr()->GetAxis(axis_name, 1, Axes[1]);
  if (Axes[0] != 0.0f || Axes[1] != 0.0f) OnAxisEvent_Implementation(Axes);

  bool TriggerPressed = false, TriggerReleased = false;
  IDisplayCluster::Get().GetInputMgr()->WasButtonPressed (buttons_name, 0, TriggerPressed );
  IDisplayCluster::Get().GetInputMgr()->WasButtonReleased(buttons_name, 0, TriggerReleased);
  if (TriggerPressed ) OnTriggerEvent_Implementation(true );
  if (TriggerReleased) OnTriggerEvent_Implementation(false);
  
  for (auto i = 1; i < 6; ++i)
  {
    bool ButtonPressed = false, ButtonReleased = false;
    IDisplayCluster::Get().GetInputMgr()->WasButtonPressed (buttons_name, i, ButtonPressed );
    IDisplayCluster::Get().GetInputMgr()->WasButtonReleased(buttons_name, i, ButtonReleased);
    if (ButtonPressed ) OnButtonEvent_Implementation(true , i);
    if (ButtonReleased) OnButtonEvent_Implementation(false, i);
  }
}
