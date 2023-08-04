// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractableBase.h"

#include "Interaction/ClickBehaviour.h"
#include "Interaction/HoverBehaviour.h"

// Sets default values for this component's properties
UInteractableBase::UInteractableBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UInteractableBase::RestrictInteractionToComponents(TArray<USceneComponent*> Components)
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
	// ...
	
}

// Called every frame
void UInteractableBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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
	//Selecting
	TInlineComponentArray<UHoverBehaviour*> AttachedHoverBehaviours;
	GetOwner()->GetComponents(AttachedHoverBehaviours, true);

	OnHoverBehaviours = AttachedHoverBehaviours;

	//Clicking
	TInlineComponentArray<UClickBehaviour*> AttachedClickBehaviours;
	GetOwner()->GetComponents(AttachedClickBehaviours, true);

	OnClickBehaviours = AttachedClickBehaviours;

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

