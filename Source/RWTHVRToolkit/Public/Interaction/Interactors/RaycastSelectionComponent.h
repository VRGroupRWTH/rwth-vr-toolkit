// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Pawn/InputExtensionInterface.h>

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RaycastSelectionComponent.generated.h"


UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API URaycastSelectionComponent : public USceneComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URaycastSelectionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMCRaycastSelection;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* RayCastSelectInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
	float TraceLength = 3000.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
	bool bShowDebugTrace = false;

private:
	UFUNCTION()
	void OnBeginSelect(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndSelect(const FInputActionValue& Value);

public:
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	UInteractableComponent* PreviousInteractable;

	UPROPERTY()
	UInteractableComponent* CurrentInteractable;
};
