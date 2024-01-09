// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Pawn/InputExtensionInterface.h>

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RaycastInteractionComponent.generated.h"


UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API URaycastInteractionComponent : public USceneComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URaycastInteractionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* InteractionInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
	float TraceLength = 3000.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
	bool bShowDebugTrace = false;

private:
	UFUNCTION()
	void OnBeginInteraction(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndInteraction(const FInputActionValue& Value);

public:
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	UInteractableComponent* PreviousInteractable;

	UPROPERTY()
	UInteractableComponent* CurrentInteractable;
};
