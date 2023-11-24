// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/ActionBehaviour.h"
#include "Interaction/Interactables/HoverBehaviour.h"

void UInteractableComponent::RestrictInteractionToComponents(const TArray<USceneComponent*>& Components)
{
	if (Components.IsEmpty())
	{
		bRestrictInteraction = false;
		AllowedComponents.Empty();
	}
	else
	{
		bRestrictInteraction = true;
		AllowedComponents = Components;
	}
}

void UInteractableComponent::RestrictInteractionToComponent(USceneComponent* Component)
{
	TArray<USceneComponent*> Components;
	Components.Add(Component);
	RestrictInteractionToComponents(Components);
}

void UInteractableComponent::ResetRestrictInteraction()
{
	bRestrictInteraction = false;
	AllowedComponents.Empty();
}

// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	InitDefaultBehaviourReferences();
}

// This functions dispatches the HoverStart Event to the attached Hover Behaviour Components
void UInteractableComponent::HandleOnHoverStartEvents(USceneComponent* TriggerComponent,
													  const EInteractorType Interactor)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all HoverBehaviours
	for (const UHoverBehaviour* b : OnHoverBehaviours)
	{
		b->OnHoverStartEvent.Broadcast(TriggerComponent, HitResult);
	}
}

// This functions dispatches the HoverEnd Event to the attached Hover Behaviour Components
void UInteractableComponent::HandleOnHoverEndEvents(USceneComponent* TriggerComponent, const EInteractorType Interactor)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all HoverBehaviours
	for (const UHoverBehaviour* b : OnHoverBehaviours)
	{
		b->OnHoverEndEvent.Broadcast(TriggerComponent);
	}
}

// This functions dispatches the ActionStart Event to the attached Action Behaviour Components
void UInteractableComponent::HandleOnActionStartEvents(USceneComponent* TriggerComponent,
													   const UInputAction* InputAction, const FInputActionValue& Value,
													   const EInteractorType Interactor)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all ActionBehaviours
	for (const UActionBehaviour* b : OnActionBehaviours)
	{
		b->OnActionBeginEvent.Broadcast(TriggerComponent, InputAction, Value);
	}
}

// This functions dispatches the ActionEnd Event to the attached Action Behaviour Components
void UInteractableComponent::HandleOnActionEndEvents(USceneComponent* TriggerComponent, const UInputAction* InputAction,
													 const FInputActionValue& Value, const EInteractorType Interactor)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all ActionBehaviours
	for (const UActionBehaviour* b : OnActionBehaviours)
	{
		b->OnActionEndEvent.Broadcast(TriggerComponent, InputAction, Value);
	}
}

// This function searches for Action and Hover Behaviours that are attached to the Actor iff they are not set
// manually by the user.
void UInteractableComponent::InitDefaultBehaviourReferences()
{
	// only do this if empty, otherwise the user has explicitly stated, which behaviors to include
	if (OnHoverBehaviours.IsEmpty())
	{
		// Selecting
		TInlineComponentArray<UHoverBehaviour*> AttachedHoverBehaviours;
		GetOwner()->GetComponents(AttachedHoverBehaviours, true);

		OnHoverBehaviours = AttachedHoverBehaviours;
	}

	// only do this if empty, otherwise the user has explicitly stated, which behaviors to include
	if (OnActionBehaviours.IsEmpty())
	{
		// Clicking
		TInlineComponentArray<UActionBehaviour*> AttachedClickBehaviours;
		GetOwner()->GetComponents(AttachedClickBehaviours, true);

		OnActionBehaviours = AttachedClickBehaviours;
	}
}

bool UInteractableComponent::IsComponentAllowed(USceneComponent* Component) const
{
	if (bRestrictInteraction)
	{
		if (!AllowedComponents.Contains(Component))
		{
			return false;
		}
	}
	return true;
}
