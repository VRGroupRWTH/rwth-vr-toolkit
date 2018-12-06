#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CoreMinimal.h"
#include "DisplayClusterPawn.h"
#include "DisplayClusterSceneComponent.h"
#include "MotionControllerComponent.h"

#include "VirtualRealityPawn.generated.h"

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API AVirtualRealityPawn : public ADisplayClusterPawn
{
  GENERATED_UCLASS_BODY()

public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnForward   (float Value  );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRight     (float Value  );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnTurnRate  (float Rate   );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnLookUpRate(float Rate   );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnFire      (bool  Pressed);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnAction    (bool  Pressed, int32 Index);

protected:
  DECLARE_DELEGATE_OneParam (FFireDelegate  , bool);
  DECLARE_DELEGATE_TwoParams(FActionDelegate, bool, int32);

  virtual void                    BeginPlay                ()                                            override;
  virtual void                    Tick                     (float            DeltaSeconds        )       override;
  virtual void                    BeginDestroy             ()                                            override;
  virtual void                    SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)       override;
  virtual UPawnMovementComponent* GetMovementComponent     ()                                      const override;
  
  UPROPERTY(EditAnywhere   , BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true")) float                          BaseTurnRate          = 45.0f  ;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UFloatingPawnMovement*         Movement              = nullptr;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) URotatingMovementComponent*    RotatingMovement      = nullptr;
  
  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. CAVE/ROLV flystick.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UDisplayClusterSceneComponent* Flystick              = nullptr;
  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD left  motion controller.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UMotionControllerComponent*    LeftMotionController  = nullptr;
  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD right motion controller.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UMotionControllerComponent*    RightMotionController = nullptr;

  // PC: RootComponent, HMD: LeftMotionController , CAVE/ROLV: Flystick. Movement follows this component.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*               Forward               = nullptr;
  // PC: RootComponent, HMD: LeftMotionController , CAVE/ROLV: Flystick. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*               LeftHand              = nullptr;
  // PC: RootComponent, HMD: RightMotionController, CAVE/ROLV: Flystick. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*               RightHand             = nullptr;
};
