// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"
#include "Pawn/VirtualRealityPawn.h"

#include "ContinuousMovementComponent.generated.h"


/*
 * This Movement component is needed since in VR not only the pawn itself (UpdatedComponent) is moved but also the
 * user herself can walk and thereby move the CameraComponent, which can also lead to collisions or e.g. going up steps 
 *
 * The four modes are:
 * None: No controller movement is applied and no corrections regarding steps or collisions with walls are done
 * Ghost: The same as above but now the Inputs can be used for unconstrained flying (also through objects)
 * Fly: The user can fly but not through walls etc. When the user walks against a wall the scene is moved with her to avoid walking through
 *      The user can also walk up stairs with a maximum step height of MaxStepHeight
 * Walk: Additionally to Fly now gravity keeps the user on the floor
 */

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8
{
	NAV_NONE UMETA(DisplayName = "None (no controller movement)"),
	NAV_GHOST UMETA(DisplayName = "Ghost (flying, also through walls)"),
	NAV_FLY UMETA(DisplayName = "Fly (prohibiting collisions)"),
	NAV_WALK UMETA(DisplayName = "Walk (gravity and prohibiting collisions)")
};


/**
 * 
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UContinuousMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:

	UContinuousMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;
	
	void SetHeadComponent(USceneComponent* NewHeadComponent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	EVRNavigationModes NavigationMode = EVRNavigationModes::NAV_WALK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float MaxStepHeight = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float GravityAcceleration = 981.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float UpSteppingAcceleration = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float CapsuleRadius = 40.0f;


	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnBeginMove(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementRight;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementLeft;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UVRPawnInputConfig* InputActions;
	
	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION()
	void StartDesktopRotation();
	
	UFUNCTION()
	void EndDesktopRotation();
	
	bool bApplyDesktopRotation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bSnapTurn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta=(EditCondition="!bSnapTurn"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta=(EditCondition="bSnapTurn",ClampMin=0,ClampMax=360))
	float SnapTurnAngle = 22.5;

private:
	FHitResult CreateLineTrace(FVector Direction, const FVector Start, bool Visibility);
	FHitResult CreateMultiLineTrace(FVector Direction, const FVector Start, float Radius, bool Visibility);
	void SetCapsuleColliderToUserSize();
	void CheckForPhysWalkingCollision();
	bool CheckForVirtualMovCollision(FVector PositionChange, float DeltaTime);
	void MoveByGravityOrStepUp(float DeltaSeconds);
	void ShiftVertically(float DiffernceDistance, float VerticalAcceleration, float DeltaSeconds, int Direction);
	//(direction = Down = -1), (direction = Up = 1)

	UPROPERTY(VisibleAnywhere) UCapsuleComponent* CapsuleColliderComponent = nullptr;
	UPROPERTY() USceneComponent* HeadComponent = nullptr;

	float VerticalSpeed = 0.0f;
	FVector LastHeadPosition;


	UPROPERTY()
	UUniversalTrackedComponent* MovementHand;
	
	UPROPERTY()
	UUniversalTrackedComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCMovement;

	void SetupInputActions();

	AVirtualRealityPawn* VRPawn;

	/*virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;*/

	/**
	* Fixes camera rotation in desktop mode.
	*/
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();
	
};
