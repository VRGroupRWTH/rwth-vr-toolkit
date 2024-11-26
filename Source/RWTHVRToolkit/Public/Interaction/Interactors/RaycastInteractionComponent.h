// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UBaseInteractionComponent.h"
#include "Components/SceneComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "RaycastInteractionComponent.generated.h"


UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API URaycastInteractionComponent : public UUBaseInteractionComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URaycastInteractionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
	float TraceLength = 3000.0;

private:
	virtual void OnBeginInteractionInputAction(const FInputActionValue& Value) override;

	virtual void OnEndInteractionInputAction(const FInputActionValue& Value) override;

public:
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	UInteractableComponent* PreviousInteractable;

	UPROPERTY()
	UInteractableComponent* CurrentInteractable;
};
