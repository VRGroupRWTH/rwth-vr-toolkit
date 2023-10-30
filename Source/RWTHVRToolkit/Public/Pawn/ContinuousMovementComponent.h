// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Pawn/MovementComponentBase.h"
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
class RWTHVRTOOLKIT_API UContinuousMovementComponent : public UMovementComponentBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	EVRSteeringModes SteeringMode = EVRSteeringModes::STEER_HAND_DIRECTED;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	UInputMappingContext* IMCMovementLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	UInputMappingContext* IMCMovementRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	UInputAction* Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	UInputAction* MoveUp;

	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnMove(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnMoveUp(const FInputActionValue& Value);

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	class UInputMappingContext* IMCMovement;

	UPROPERTY()
	UMotionControllerComponent* MovementHand;

	UPROPERTY()
	UMotionControllerComponent* RotationHand;
};
