// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementComponentBase.h"
#include "TurnComponent.generated.h"

class UMotionControllerComponent;

UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UTurnComponent : public UMovementComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bTurnWithLeftHand = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bAllowTurning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning",
			  meta = (EditCondition = "bAllowTurning"))
	bool bSnapTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning",
			  meta = (EditCondition = "!bSnapTurn && bAllowTurning"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning",
			  meta = (EditCondition = "bSnapTurn && bAllowTurning", ClampMin = 0, ClampMax = 360))
	float SnapTurnAngle = 22.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* XRTurn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopTurn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopTurnCondition;

	/**
	 * Called every tick as long as stick input is received to allow for continuous turning
	 * @param Value Stick input value determines turn direction and turn speed
	 */
	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	/**
	 * Called once if stick input is received to rotate player at constant value
	 * @param Value Stick input value determines turn direction
	 */
	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);


	// A separate button has to be pressed, for when mouse movement should contribute to turning
	UFUNCTION()
	void StartDesktopRotation();

	UFUNCTION()
	void EndDesktopRotation();

private:
	UPROPERTY()
	UMotionControllerComponent* RotationHand;

	/**
	 * If we just use VRPawn->AddControllerYawInput(Yaw), rotation is around tracking origin instead of the actual
	 * player position This function updates the pawns rotation and location to result in a rotation around the users
	 * tracked position.
	 */
	void RotateCameraAndPawn(float Yaw) const;
	bool bApplyDesktopRotation;
};
