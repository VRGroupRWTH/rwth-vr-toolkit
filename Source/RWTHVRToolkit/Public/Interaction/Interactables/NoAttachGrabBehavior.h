// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBehaviour.h"
#include "Components/SceneComponent.h"
#include "Interaction/Interactables/GrabBehavior.h"
#include "NoAttachGrabBehavior.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UNoAttachGrabBehavior : public UActionBehaviour
{
	GENERATED_BODY()

public:
	UNoAttachGrabBehavior();

	UPROPERTY(EditAnywhere, Category = "Grabbing")
	bool bBlockOtherInteractionsWhileGrabbed = true;

	virtual void OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							   const FInputActionValue& Value) override;

	virtual void OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
							 const FInputActionValue& Value) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

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

	UPROPERTY()
	USceneComponent* ProxyComponent;

	UFUNCTION(BlueprintPure)
	bool IsObjectGrabbed() const { return bObjectGrabbed; }

	UPROPERTY(EditAnywhere)
	bool bSyncLocation = true;

	UPROPERTY(EditAnywhere)
	bool bSyncRotation = true;

	UPROPERTY(EditAnywhere)
	bool bAffectsParentActor = false;

	UPROPERTY(VisibleAnywhere)
	AActor* ParentActor;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* CurrentAttachParent;

private:
	/**
	 * Try to detach the object from the hand. Keep this private for now as this does not broadcast the GrabEnd Event
	 * correctly.
	 * @return true if object was successfully detached. If detachment failed or if object was not grabbed before,
	 * return false.
	 */
	bool TryRelease();

	bool bObjectGrabbed = false;

	bool bWasSimulatingPhysics;
};
