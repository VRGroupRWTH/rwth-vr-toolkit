#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CoreMinimal.h"
#include "DisplayClusterPawn.h"
#include "DisplayClusterSceneComponent.h"

#include "DisplayClusterPawnCAVE.generated.h"

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API ADisplayClusterPawnCAVE : public ADisplayClusterPawn
{
  GENERATED_UCLASS_BODY()

public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnForward   (float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRight     (float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnTurnRate  (float Rate );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnLookUpRate(float Rate );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnFire      ();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnAction    (int32 Index);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") float                          BaseTurnRate = 45.0f  ;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") UDisplayClusterSceneComponent* Flystick     = nullptr;

protected:
  DECLARE_DELEGATE_OneParam(FButtonDelegate, int32);

  virtual void                    BeginPlay                ()                                            override;
  virtual void                    Tick                     (float            DeltaSeconds        )       override;
  virtual void                    BeginDestroy             ()                                            override;
  virtual void                    SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)       override;
  virtual UPawnMovementComponent* GetMovementComponent     ()                                      const override;
  
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UFloatingPawnMovement*      MovementComponent = nullptr;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) URotatingMovementComponent* RotatingComponent = nullptr;
};
