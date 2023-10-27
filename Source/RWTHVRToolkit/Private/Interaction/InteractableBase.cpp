// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractableBase.h"

#include "Interaction/ClickBehaviour.h"
#include "Interaction/HoverBehaviour.h"

void UInteractableBase::RestrictInteractionToComponents(const TArray<USceneComponent*>& Components)
{
	if(Components.IsEmpty())
	{
		bRestrictInteraction = false;
		AllowedComponents.Empty();
	} else
	{
		bRestrictInteraction = true;
		AllowedComponents = Components;
	}
}

void UInteractableBase::RestrictInteractionToComponent(USceneComponent* Component)
{
	TArray<USceneComponent*> Components;
	Components.Add(Component);
	RestrictInteractionToComponents(Components);
}

void UInteractableBase::ResetRestrictInteraction()
{
	bRestrictInteraction = false;
	AllowedComponents.Empty();
}

// Called when the game starts
void UInteractableBase::BeginPlay()
{
	Super::BeginPlay();
	InitDefaultBehaviourReferences();
}

void UInteractableBase::HandleOnHoverStartEvents(USceneComponent* TriggerComponent)
{
	if(!IsComponentAllowed(TriggerComponent)) return;
	
	for(const UHoverBehaviour* b : OnHoverBehaviours)
	{
		b->OnHoverStartEvent.Broadcast(TriggerComponent, HitResult);
	}

}

void UInteractableBase::HandleOnHoverEndEvents(USceneComponent* TriggerComponent)
{
	if(!IsComponentAllowed(TriggerComponent)) return;
	
	for(const UHoverBehaviour* b : OnHoverBehaviours)
	{
		b->OnHoverEndEvent.Broadcast(TriggerComponent);
	}
}

void UInteractableBase::HandleOnClickStartEvents(USceneComponent* TriggerComponent, const FInputActionValue& Value)
{
	if(!IsComponentAllowed(TriggerComponent)) return;
	
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		b->OnClickStartEvent.Broadcast(TriggerComponent, Value);
	}
}

void UInteractableBase::HandleOnClickEndEvents(USceneComponent* TriggerComponent, const FInputActionValue& Value)
{
	if(!IsComponentAllowed(TriggerComponent)) return;
	
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		b->OnClickEndEvent.Broadcast(TriggerComponent, Value);
	}
}

void UInteractableBase::InitDefaultBehaviourReferences()
{
	// only do this if empty, otherwise the user has explicitly stated, which behaviors to include
	if(OnHoverBehaviours.IsEmpty())
	{
		//Selecting
		TInlineComponentArray<UHoverBehaviour*> AttachedHoverBehaviours;
		GetOwner()->GetComponents(AttachedHoverBehaviours, true);

		OnHoverBehaviours = AttachedHoverBehaviours;
	}
	
	// only do this if empty, otherwise the user has explicitly stated, which behaviors to include
	if(OnClickBehaviours.IsEmpty())
	{
		//Clicking
		TInlineComponentArray<UClickBehaviour*> AttachedClickBehaviours;
		GetOwner()->GetComponents(AttachedClickBehaviours, true);

		OnClickBehaviours = AttachedClickBehaviours;
	}
}

bool UInteractableBase::IsComponentAllowed(USceneComponent* Component) const
{
	if(bRestrictInteraction)
	{
		if(!AllowedComponents.Contains(Component))
		{
			return false;
		} 
	}
	return true;
}

