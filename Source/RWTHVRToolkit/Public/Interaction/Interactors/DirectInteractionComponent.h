// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "DirectInteractionComponent.generated.h"

UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API UDirectInteractionComponent : public USceneComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDirectInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* InteractionInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	float InteractionSphereRadius = 15.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	bool bShowDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	bool bOnlyInteractWithClosestActor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Interaction")
	TArray<AActor*> ActorsToIgnore;

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
	void OnBeginInteraction(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndInteraction(const FInputActionValue& Value);

	UPROPERTY()
	TArray<UInteractableComponent*> PreviousInteractableComponentsInRange;

	UPROPERTY()
	TArray<UInteractableComponent*> CurrentInteractableComponentsInRange;

	TArray<TWeakObjectPtr<UInteractableComponent>> CurrentlyInteractedComponents;

	UInteractableComponent* SearchForInteractable(AActor* HitActor);

	bool bSearchAtParent = false;
};
