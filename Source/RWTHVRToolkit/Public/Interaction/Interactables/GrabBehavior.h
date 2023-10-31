// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBehaviour.h"
#include "Components/SceneComponent.h"
#include "GrabBehavior.generated.h"


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

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UPROPERTY()
	UPrimitiveComponent* MyPhysicsComponent;
};
