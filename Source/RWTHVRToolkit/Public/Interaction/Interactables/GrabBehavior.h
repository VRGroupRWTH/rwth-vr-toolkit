// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBehaviour.h"
#include "Components/SceneComponent.h"
#include "GrabBehavior.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabStart, USceneComponent*, NewAttachParent, UPrimitiveComponent*,
											 HeldComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabEnd, USceneComponent*, PreviousAttachParent, UPrimitiveComponent*,
											 HeldComponent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UGrabBehavior : public UActionBehaviour
{
	GENERATED_BODY()

public:
	UGrabBehavior();

	UPROPERTY(EditAnywhere, Category = "Grabbing")
	bool bBlockOtherInteractionsWhileGrabbed = true;

	UPROPERTY(EditAnywhere, Category = "Grabbing")
	bool bIgnoreGrabbedActorInCollisionMovement = true;

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
	UPrimitiveComponent* MyPhysicsComponent;

	virtual void BeginPlay() override;

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UFUNCTION()
	void ReplicationOriginaterClientCallback(USceneComponent* TriggerComponent, const EInteractionEventType EventType,
											 const FInputActionValue& Value);

	void HandleCollisionHandlingMovement(const USceneComponent* CurrentAttachParent,
										 const EInteractionEventType EventType);

	virtual void OnActionEvent(USceneComponent* TriggerComponent, const EInteractionEventType EventType,
							   const FInputActionValue& Value) override;

	UFUNCTION(BlueprintPure)
	bool IsObjectGrabbed() const { return bObjectGrabbed; }

private:
	/**
	 * Try to detach the object from the hand. Keep this private for now as this does not broadcast the GrabEnd Event
	 * correctly.
	 * @return true if object was successfully detached. If detachment failed or if object was not grabbed before,
	 * return false.
	 */
	bool TryRelease();

	void StartGrab(USceneComponent* TriggerComponent);

	void EndGrab(USceneComponent* TriggerComponent);

	bool bObjectGrabbed = false;

	bool bWasSimulatingPhysics;

	bool bWasAddedToIgnore = false;
};
