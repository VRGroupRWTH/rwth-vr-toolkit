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
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCallable, Category = "Pawn") void OnFire(bool Pressed);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnAction(bool Pressed, int32 Index);

  UFUNCTION(BlueprintPure, Category = "Pawn") static bool IsDesktopMode();
  UFUNCTION(BlueprintPure, Category = "Pawn") static bool IsRoomMountedMode();
  UFUNCTION(BlueprintPure, Category = "Pawn") static bool IsHeadMountedMode();

  UFUNCTION(BlueprintPure, Category = "Pawn") static FString GetNodeName();
  UFUNCTION(BlueprintPure, Category = "Pawn") static float GetEyeDistance();

  UFUNCTION(Category = "Pawn") float GetBaseTurnRate() const;
  UFUNCTION(Category = "Pawn") void SetBaseTurnRate(float Value);
  UFUNCTION(Category = "Pawn") UFloatingPawnMovement* GetFloatingPawnMovement();
  UFUNCTION(Category = "Pawn") URotatingMovementComponent* GetRotatingMovementComponent();

  //Bunch of Getter Functions for components to avoid users having to know the names

  UFUNCTION(Category = "Pawn") UDisplayClusterSceneComponent* GetFlystickComponent();
  UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdLeftMotionControllerComponent();
  UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdRightMotionControllerComponent();

  UFUNCTION(Category = "Pawn") USceneComponent* GetHeadComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetLeftHandComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetRightHandComponent();

  UFUNCTION(Category = "Pawn") USceneComponent* GetCaveOriginComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetCaveCenterComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetShutterGlassesComponent();

  //Get Compenent of Display Cluster by it's name, which is specified in the nDisplay config
  UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Pawn") static UDisplayClusterSceneComponent* GetClusterComponent(const FString& Name);
   
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EVRNavigationModes NavigationMode = EVRNavigationModes::NAV_MODE_FLY;

protected:
	DECLARE_DELEGATE_OneParam(FFireDelegate, bool);
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
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UMotionControllerComponent*    HmdLeftMotionController  = nullptr;
  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD right motion controller.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Pawn", meta = (AllowPrivateAccess = "true")) UMotionControllerComponent*    HmdRightMotionController = nullptr;

  // PC: Camera, HMD: Camera, CAVE/ROLV: Shutter glasses.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))  USceneComponent*				Head						= nullptr;
  // PC: RootComponent, HMD: HmdLeftMotionController , CAVE/ROLV: Flystick. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))  USceneComponent*				LeftHand					= nullptr;
  // PC: RootComponent, HMD: HmdRightMotionController, CAVE/ROLV: Flystick. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))  USceneComponent*				RightHand					= nullptr;

  // Holding the Cave Origin Component that is attached to this Pawn
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*				CaveOrigin					= nullptr;
  // Holding the Cave Center Component that is attached to this Pawn
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*				CaveCenter					= nullptr;
  // Holding the Shutter Glasses Component that is attached to this Pawn
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent*				ShutterGlasses				= nullptr;

private:
	void InitComponentReferences();
};
