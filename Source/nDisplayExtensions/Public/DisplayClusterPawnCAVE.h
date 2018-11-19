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
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input") void OnAxisEvent   (FVector2D Value  );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input") void OnButtonEvent (bool      Pressed, int32 Index);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input") void OnTriggerEvent(bool      Pressed);
  
protected:
  virtual void Tick (float DeltaSeconds) override;

  UDisplayClusterSceneComponent* Flystick = nullptr;
};
