// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBehaviour.h"
#include "Components/SceneComponent.h"
#include "GrabBehavior.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabStart, USceneComponent*, Hand, UPrimitiveComponent*, HeldComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabEnd, USceneComponent*, Hand, UPrimitiveComponent*, HeldComponent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UGrabBehavior : public UActionBehaviour
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Grabbing")
	bool bBlockOtherInteractionsWhileGrabbed = true;

	virtual void OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							   const FInputActionValue& Value) override;
	virtual void OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							 const FInputActionValue& Value) override;

	/**
	 * Called after the object was successfully attached to the hand
	 */
	UPROPERTY(BlueprintAssignable)
	FOnGrabStart OnGrabStartEvent;

	/**
	 * Called after the object was successfully detached from the hand
	 */
	UPROPERTY(BlueprintAssignable)
	FOnGrabEnd OnGrabEndEvent;

	/**
	 * Try to detach the object from the hand.
	 * @return true if object was successfully detached. If detachment failed or if object was not grabbed before, return false.
	 */

	UFUNCTION(BlueprintCallable)
	bool TryRelease();

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UPROPERTY()
	UPrimitiveComponent* MyPhysicsComponent;

	UFUNCTION(BlueprintPure)
	bool IsObjectGrabbed() const
	{
		return bObjectGrabbed;
	}

private:
	UPROPERTY()
	bool bObjectGrabbed = false;

	UPROPERTY()
	bool bWasSimulatingPhysics;
};
