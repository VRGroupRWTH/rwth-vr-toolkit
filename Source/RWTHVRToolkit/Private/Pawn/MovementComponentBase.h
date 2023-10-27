// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementComponentBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UMovementComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bAllowTurning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta = (EditCondition = "bAllowTurning"))
	bool bSnapTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta = (EditCondition = "!bSnapTurn && bAllowTurning"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta = (EditCondition = "bSnapTurn && bAllowTurning", ClampMin = 0, ClampMax = 360))
	float SnapTurnAngle = 22.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopRotation;

	
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

protected:
	void SetupInputActions();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCRotation;

private:


	bool bApplyDesktopRotation = false;
	
};
