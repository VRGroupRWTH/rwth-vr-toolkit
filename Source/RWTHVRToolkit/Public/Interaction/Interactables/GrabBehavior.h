// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBehaviour.h"
#include "Components/SceneComponent.h"
#include "GrabBehavior.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabStart, UPrimitiveComponent*, HeldComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabEnd, UPrimitiveComponent*, HeldComponent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UGrabBehavior : public UActionBehaviour
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Grabbing")
	bool bBlockOtherInteractionsWhileGrabbed = true;

	virtual void OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
	                           const FInputActionValue& Value) override;
	virtual void OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
	                         const FInputActionValue& Value) override;


	/**
	* TriggeredComponent: Component that triggered this event (e.g. GrabComponent, RayCastComponent attached at the VRPawn)
	* Hit: Hit Result of the trace to get access to e.g. contact point/normals etc.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGrabStart OnGrabStartEvent;

	UPROPERTY(BlueprintAssignable)
	FOnGrabEnd OnGrabEndEvent;
	
	UFUNCTION(BlueprintCallable)
	bool TryRelease();

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UPROPERTY()
	UPrimitiveComponent* MyPhysicsComponent;

	UFUNCTION(BlueprintCallable)
	bool IsObjectGrabbed()
	{
		return bObjectGrabbed;
	}

private:
	UPROPERTY()
	bool bObjectGrabbed = false;
};
