// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Components/ActorComponent.h"
#include "ContinuousMovementComponent.generated.h"


UENUM(BlueprintType)
enum class EVRSteeringModes : uint8
{
	STEER_GAZE_DIRECTED UMETA(DisplayName = "Gaze Directed (Movement input is applied in head component direction)"),
	STEER_HAND_DIRECTED UMETA(DisplayName = "Hand Direced (Movement input is applied in controller direction)")
};

/**
 * 
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UContinuousMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	EVRSteeringModes SteeringMode = EVRSteeringModes::STEER_HAND_DIRECTED;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bSnapTurn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta=(EditCondition="!bSnapTurn"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta=(EditCondition="bSnapTurn",ClampMin=0,ClampMax=360))
	float SnapTurnAngle = 22.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementRight;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementLeft;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UVRPawnInputConfig* InputActions;
	


	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnBeginMove(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginUp(const FInputActionValue& Value);
	
	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION()
	void StartDesktopRotation();
	
	UFUNCTION()
	void EndDesktopRotation();
	
	bool bApplyDesktopRotation = false;

private:
	
	
	UPROPERTY()
	UUniversalTrackedComponent* MovementHand;
	
	UPROPERTY()
	UUniversalTrackedComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCMovement;

	void SetupInputActions();

	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	/**
	* Fixes camera rotation in desktop mode.
	*/
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();
	
};
