// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementComponentBase.h"
#include "TurnComponent.generated.h"


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
	class UInputAction* Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopRotation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovement_Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovement_Right;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCDesktopRotation;

	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);

	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION()
	void StartDesktopRotation();

	UFUNCTION()
	void EndDesktopRotation();

private:
	UPROPERTY()
	UMotionControllerComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCTurn;

	/**
	 * If we just use VRPawn->AddControllerYawInput(Yaw), rotation is around tracking origin instead of the actual player position
	 * This function updates the pawns rotation and location to result in a rotation around the users tracked position.
	 */
	void RotateCameraAndPawn(float Yaw);
	bool bApplyDesktopRotation;
};
