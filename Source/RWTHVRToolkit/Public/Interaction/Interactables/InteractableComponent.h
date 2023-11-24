// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InteractionBitSet.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"


struct FInputActionValue;
class UActionBehaviour;
class UHoverBehaviour;

/*
 * This class is the entry point into interactions on actors.
 * The InteractorComponents will call this component to notify it about Action and Hover events.
 *
 * The Interactable component receives Action and Hover calls and broadcasts it to all registered
 * Action and Hover Behaviour components.
 *
 * The Interactable can be filtered.
 * Currently we filter on the InteractorType, e.g.: Raycast, Spherecast, etc.
 *
 * Action and Hover Behaviours can be manually added via Blueprints on a per-Interactable basis.
 * If no Action and Hover Behaviours are set manually, all Action and Hover behaviours that are
 * attached to the Actor are used.
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsInteractable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/RWTHVRToolkit.EInteractorType"))
	int32 InteractorFilter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UHoverBehaviour*> OnHoverBehaviours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UActionBehaviour*> OnActionBehaviours;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasInteractionTypeFlag(EInteractorType type) { return type & InteractorFilter; }

	/**
	 * @brief Restrict interactability to given components (e.g. if an object is grabbed, block interactions from other
	 * components)
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

	void HandleOnHoverStartEvents(USceneComponent* TriggerComponent, const EInteractorType Interactor);
	void HandleOnHoverEndEvents(USceneComponent* TriggerComponent, const EInteractorType Interactor);
	void HandleOnActionStartEvents(USceneComponent* TriggerComponent, const UInputAction* InputAction,
								   const FInputActionValue& Value, const EInteractorType Interactor);
	void HandleOnActionEndEvents(USceneComponent* TriggerComponent, const UInputAction* InputAction,
								 const FInputActionValue& Value, const EInteractorType Interactor);

	/**
	 * @brief If hover and action behaviors are not explicitly specified, load all existing ones
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
};
