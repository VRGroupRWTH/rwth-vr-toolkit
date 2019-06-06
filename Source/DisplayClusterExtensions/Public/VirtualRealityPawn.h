#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CoreMinimal.h"
#include "DisplayClusterPawn.h"
#include "DisplayClusterSceneComponent.h"
#include "MotionControllerComponent.h"

#include "VirtualRealityPawn.generated.h"

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8{
  NAV_MODE_NONE UMETA(DisplayName = "Navigation Mode None"),
  NAV_MODE_FLY UMETA(DisplayName = "Navigation Mode Fly")
};

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API AVirtualRealityPawn : public ADisplayClusterPawn
{
  GENERATED_UCLASS_BODY()

public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnForward   (float Value  );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRight     (float Value  );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnTurnRate  (float Rate   );
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnLookUpRate(float Rate   );

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EVRNavigationModes NavigationMode = EVRNavigationModes::NAV_MODE_FLY;

protected:

  DECLARE_DELEGATE_OneParam(FAxisDelegate, float);

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
