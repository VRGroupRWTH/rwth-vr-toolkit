#pragma once

#include "CoreMinimal.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "DisplayClusterPawn.h"
#include "DisplayClusterSceneComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "MotionControllerComponent.h"
#include "VirtualRealityPawn.generated.h"

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8
{
	nav_mode_none UMETA(DisplayName = "Navigation Mode None"),
	nav_mode_fly UMETA(DisplayName = "Navigation Mode Fly")
};

UENUM(BlueprintType)
enum class EAttachementType : uint8
{
	AT_NONE UMETA(DisplayName = "not attached"),
	AT_HANDTARGET UMETA(DisplayName = "to the right/left hand target"),
	AT_FLYSTICK UMETA(DisplayName = "to the Flystick")
};

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API AVirtualRealityPawn : public ADisplayClusterPawn
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnForward(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRight(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnTurnRate(float Rate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnLookUpRate(float Rate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCallable, Category = "Pawn") void OnFire(bool Pressed);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnAction(bool Pressed, int32 Index);

	UFUNCTION(Category = "Pawn") float GetBaseTurnRate() const;
	UFUNCTION(Category = "Pawn") void SetBaseTurnRate(float Value);
	UFUNCTION(Category = "Pawn") UFloatingPawnMovement* GetFloatingPawnMovement();
	UFUNCTION(Category = "Pawn") URotatingMovementComponent* GetRotatingMovementComponent();

	//Bunch of Getter Functions for components to avoid users having to know the names
	UFUNCTION(Category = "Pawn") UDisplayClusterSceneComponent* GetFlystickComponent();
	UFUNCTION(Category = "Pawn") UDisplayClusterSceneComponent* GetRightHandtargetComponent();
	UFUNCTION(Category = "Pawn") UDisplayClusterSceneComponent* GetLeftHandtargetComponent();
	UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdLeftMotionControllerComponent();
	UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdRightMotionControllerComponent();

	UFUNCTION(Category = "Pawn") USceneComponent* GetHeadComponent();
	UFUNCTION(Category = "Pawn") USceneComponent* GetLeftHandComponent();
	UFUNCTION(Category = "Pawn") USceneComponent* GetRightHandComponent();

	UFUNCTION(Category = "Pawn") USceneComponent* GetTrackingOriginComponent();
private:
	UFUNCTION(Category = "Pawn") USceneComponent* GetCaveCenterComponent();
	UFUNCTION(Category = "Pawn") USceneComponent* GetShutterGlassesComponent();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EVRNavigationModes NavigationMode = EVRNavigationModes::nav_mode_fly;

  //Execute specified console command on all nDisplayCluster Nodes
  UFUNCTION(Exec, BlueprintCallable, Category = "DisplayCluster") static void ClusterExecute(const FString& Command);

private:
	FOnClusterEventListener ClusterEventListenerDelegate;
	UFUNCTION() void HandleClusterEvent(const FDisplayClusterClusterEvent& Event);

protected:
	DECLARE_DELEGATE_OneParam(FFireDelegate, bool);
	DECLARE_DELEGATE_TwoParams(FActionDelegate, bool, int32);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true")) float BaseTurnRate = 45.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UFloatingPawnMovement* Movement = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) URotatingMovementComponent* RotatingMovement = nullptr;

	// Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. CAVE/ROLV flystick.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UDisplayClusterSceneComponent* Flystick = nullptr;
	// Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. CAVE/ROLV flystick.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UDisplayClusterSceneComponent* RightHandTarget = nullptr;
	// Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. CAVE/ROLV flystick.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UDisplayClusterSceneComponent* LeftHandTarget = nullptr;

	// Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD left  motion controller.
	UMotionControllerComponent* HmdLeftMotionController = nullptr;
	// Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD right motion controller.
	UMotionControllerComponent* HmdRightMotionController = nullptr;

	// PC: Camera, HMD: Camera, CAVE/ROLV: Shutter glasses.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* Head = nullptr;
	// PC: RootComponent, HMD: HmdLeftMotionController , CAVE/ROLV: regarding to AttachRightHandInCAVE. Useful for line trace (e.g. for holding objects).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* RightHand = nullptr;
	// PC: RootComponent, HMD: HmdRightMotionController, CAVE/ROLV: regarding to AttachLeftHandInCAVE. Useful for line trace (e.g. for holding objects).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* LeftHand = nullptr;

	// Holding the Cave/rolv Origin Component that is attached to this Pawn
	UPROPERTY() USceneComponent* TrackingOrigin = nullptr;
	// Holding the Cave Center Component that is attached to this Pawn, it is needed for the internal transform of nDisplay
	UPROPERTY() USceneComponent* CaveCenter = nullptr;
	// Holding the Shutter Glasses Component that is attached to this Pawn
	UPROPERTY() USceneComponent* ShutterGlasses = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") bool ShowHMDControllers = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EAttachementType AttachRightHandInCAVE = EAttachementType::AT_FLYSTICK;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EAttachementType AttachLeftHandInCAVE = EAttachementType::AT_NONE;

private:
	void InitRoomMountedComponentReferences();
};