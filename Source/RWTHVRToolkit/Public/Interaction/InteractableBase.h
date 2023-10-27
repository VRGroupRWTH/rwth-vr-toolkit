// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableBase.generated.h"


struct FInputActionValue;
class UClickBehaviour;
class UHoverBehaviour;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UInteractableBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsInteractable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UHoverBehaviour*> OnHoverBehaviours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UClickBehaviour*> OnClickBehaviours;

	/**
	 * @brief Restrict interactability to given components (e.g. if an object is grabbed, block interactions from other components)
	 * @param Components 
	 */
	UFUNCTION()
	void RestrictInteractionToComponents(const TArray<USceneComponent*>& Components);

	UFUNCTION()
	void RestrictInteractionToComponent(USceneComponent* Component);

	UFUNCTION()
	void ResetRestrictInteraction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void HandleOnHoverStartEvents(USceneComponent* TriggerComponent);
	void HandleOnHoverEndEvents(USceneComponent* TriggerComponent);
	void HandleOnClickStartEvents(USceneComponent* TriggerComponent, const FInputActionValue& Value);
	void HandleOnClickEndEvents(USceneComponent* TriggerComponent, const FInputActionValue& Value);

	/**
	 * @brief If click and grab behaviors are not explicitly specified, load all existing ones
	 */
	void InitDefaultBehaviourReferences();

	// Hit event of component that triggered this interactable
	UPROPERTY()
	FHitResult HitResult;

	UPROPERTY()
	bool bRestrictInteraction;

	UPROPERTY()
	TArray<USceneComponent*> AllowedComponents;

	bool IsComponentAllowed(USceneComponent* Component) const;

private:
	bool bInitOnce = true;
};
