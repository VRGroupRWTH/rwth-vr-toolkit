// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UBaseInteractionComponent.h"
#include "Components/SceneComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "DirectInteractionComponent.generated.h"

UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API UDirectInteractionComponent : public UUBaseInteractionComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDirectInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	float InteractionSphereRadius = 15.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	bool bOnlyInteractWithClosestActor = false;

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	virtual void OnBeginInteractionInputAction(const FInputActionValue& Value) override;

	virtual void OnEndInteractionInputAction(const FInputActionValue& Value) override;

	UPROPERTY()
	TArray<UInteractableComponent*> PreviousInteractableComponentsInRange;

	UPROPERTY()
	TArray<UInteractableComponent*> CurrentInteractableComponentsInRange;

	TArray<TWeakObjectPtr<UInteractableComponent>> CurrentlyInteractedComponents;

	UInteractableComponent* SearchForInteractable(AActor* HitActor);

	bool bSearchAtParent = false;
};
