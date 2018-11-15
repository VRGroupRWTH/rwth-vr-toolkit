#pragma once

#include "CoreMinimal.h"
#include "DisplayClusterPawnBase.h"
#include "DisplayClusterSceneComponent.h"
#include "DisplayClusterPawnCAVE.generated.h"

UCLASS()
class NDISPLAYEXTENSIONS_API ADisplayClusterPawnCAVE : public ADisplayClusterPawnBase
{
  GENERATED_BODY()

public:
  virtual void BeginPlay() override;
  virtual void MoveForward(float value) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pawn")
  UDisplayClusterSceneComponent* Flystick = nullptr;
};
