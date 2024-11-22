// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Interactables/NoAttachGrabBehavior.h"
#include "ReelableNoAttachGrabBehavior.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UReelableNoAttachGrabBehavior : public UNoAttachGrabBehavior
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UReelableNoAttachGrabBehavior();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* ReelInputAction;

	virtual void OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							   const FInputActionValue& Value) override;

	virtual void OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							 const FInputActionValue& Value) override;

	void ReelAction(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere)
	double ReelSpeed = 10.0;

	UPROPERTY(EditAnywhere)
	double HandDeadZone = 60.0;

private:
	double BoundingSphereRadius = 0.0;
	bool ActionBound = false;
};
