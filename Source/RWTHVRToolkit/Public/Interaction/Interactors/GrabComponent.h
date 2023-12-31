// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "GrabComponent.generated.h"

class UGrabbableComponent;

UCLASS(Abstract, Blueprintable)
class RWTHVRTOOLKIT_API UGrabComponent : public USceneComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGrabComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* GrabInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	float GrabSphereRadius = 15.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	bool bShowDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	bool bOnlyGrabClosestActor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	TArray<AActor*> ActorsToIgnore;

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
	void OnBeginGrab(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndGrab(const FInputActionValue& Value);

	UPROPERTY()
	TArray<UInteractableComponent*> PreviousGrabBehavioursInRange;

	UPROPERTY()
	TArray<UInteractableComponent*> CurrentGrabBehavioursInRange;

	TArray<TWeakObjectPtr<UInteractableComponent>> CurrentlyGrabbedComponents;

	UInteractableComponent* SearchForInteractable(AActor* HitActor);

	bool bSearchAtParent = false;
};
