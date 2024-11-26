// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/ActionBehaviour.h"
#include "Interaction/Interactables/HoverBehaviour.h"
#include "Interaction/Interactors/UBaseInteractionComponent.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

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

void UInteractableComponent::HandleOnHoverEvents(USceneComponent* TriggerComponent, const EInteractorType Interactor,
												 const EInteractionEventType EventType)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all HoverBehaviours
	for (UHoverBehaviour* HoverBehaviour : OnHoverBehaviours)
	{
		// Check if we need to replicate the action to the server
		if (HoverBehaviour->bExecuteOnServer)
		{
			// Request the server to execute our behaviour.
			// As we can only execute RPCs from an owned object, and we don't necessarily own this actor, pipe it
			// through the interactor. Because behaviours can also be triggered from non-interactors, only do this on a
			// successful cast

			auto* InteractorComponent = Cast<UUBaseInteractionComponent>(TriggerComponent);
			if (!InteractorComponent)
			{
				UE_LOGFMT(Toolkit, Warning,
						  "Interaction: TriggerComponent {TriggerComponent} is not a UUBaseInteractionComponent. Only "
						  "UUBaseInteractionComponent TriggerComponents can be replicated.",
						  TriggerComponent->GetName());
				return;
			}
			InteractorComponent->RequestHoverBehaviourReplicationStart(HoverBehaviour, EventType, HitResult);

			// Broadcast local callback
			HoverBehaviour->OnHoverReplicationStartedOriginatorEvent.Broadcast(TriggerComponent, EventType, HitResult);
		}
		else if (HoverBehaviour->bExecuteOnAllClients)
		{
			UE_LOGFMT(Toolkit, Warning,
					  "Interaction: Behaviour {BehaviourName} has bExecuteOnAllClients=true, which requires "
					  "bExecuteOnServer also set to true, which is not the case.",
					  HoverBehaviour->GetName());
		}
		else // skip replication altogether (default case)
		{
			HoverBehaviour->OnHoverEventEvent.Broadcast(TriggerComponent, EventType, HitResult);
		}
	}
}

// This functions dispatches the ActionStart Event to the attached Action Behaviour Components
void UInteractableComponent::HandleOnActionEvents(USceneComponent* TriggerComponent, const EInteractorType Interactor,
												  const EInteractionEventType EventType, const FInputActionValue& Value)
{
	// We early return if there the InteractorFilter is set and the Interactor is allowed.
	if (!(InteractorFilter == EInteractorType::None || InteractorFilter & Interactor))
		return;

	// We early return if the source Interactor is not part of the allowed components
	if (!IsComponentAllowed(TriggerComponent))
		return;

	// Broadcast event to all ActionBehaviours
	for (UActionBehaviour* ActionBehaviour : OnActionBehaviours)
	{
		// Check if we need to replicate the action to the server
		if (ActionBehaviour->bExecuteOnServer)
		{
			// Request the server to execute our behaviour.
			// As we can only execute RPCs from an owned object, and we don't necessarily own this actor, pipe it
			// through the interactor. Because behaviours can also be triggered from non-interactors, only do this on a
			// successful cast

			auto* InteractorComponent = Cast<UUBaseInteractionComponent>(TriggerComponent);
			if (!InteractorComponent)
			{
				UE_LOGFMT(Toolkit, Warning,
						  "Interaction: TriggerComponent {TriggerComponent} is not a UUBaseInteractionComponent. Only "
						  "UUBaseInteractionComponent TriggerComponents can be replicated.",
						  TriggerComponent->GetName());
				return;
			}
			InteractorComponent->RequestActionBehaviourReplicationStart(ActionBehaviour, EventType, Value);

			// Broadcast local callback
			ActionBehaviour->OnActionReplicationStartedOriginatorEvent.Broadcast(TriggerComponent, EventType, Value);
		}
		else if (ActionBehaviour->bExecuteOnAllClients)
		{
			UE_LOGFMT(Toolkit, Warning,
					  "Interaction: Behaviour {BehaviourName} has bExecuteOnAllClients=true, which requires "
					  "bExecuteOnServer also set to true, which is not the case.",
					  ActionBehaviour->GetName());
		}
		else // skip replication altogether (default case)
		{
			ActionBehaviour->OnActionEventEvent.Broadcast(TriggerComponent, EventType, Value);
		}
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