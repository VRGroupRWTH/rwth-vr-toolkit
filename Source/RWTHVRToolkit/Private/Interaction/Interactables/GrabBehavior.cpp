// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/GrabBehavior.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonTypes.h"

UPrimitiveComponent* UGrabBehavior::GetFirstComponentSimulatingPhysics(const AActor* TargetActor)
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	// find any component that simulates physics, then traverse the hierarchy
	for (UPrimitiveComponent* const& Component : PrimitiveComponents)
	{
		if (Component->IsSimulatingPhysics())
		{
			return GetHighestParentSimulatingPhysics(Component);
		}
	}
	return nullptr;
}

// recursively goes up the hierarchy and returns the highest parent simulating physics
UPrimitiveComponent* UGrabBehavior::GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp)
{
	if (Cast<UPrimitiveComponent>(Comp->GetAttachParent()) && Comp->GetAttachParent()->IsSimulatingPhysics())
	{
		return GetHighestParentSimulatingPhysics(Cast<UPrimitiveComponent>(Comp->GetAttachParent()));
	}
	else
	{
		return Comp;
	}
}

void UGrabBehavior::OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
								  const FInputActionValue& Value)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	USceneComponent* Hand = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());

	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

	MyPhysicsComponent = GetFirstComponentSimulatingPhysics(GetOwner());

	if (MyPhysicsComponent)
	{
		MyPhysicsComponent->SetSimulatePhysics(false);
		MyPhysicsComponent->AttachToComponent(Hand, Rules);
	}
	else
	{
		GetOwner()->GetRootComponent()->AttachToComponent(Hand, Rules);
	}


	if (bBlockOtherInteractionsWhileGrabbed)
	{
		TArray<UInteractableComponent*> Interactables;
		GetOwner()->GetComponents<UInteractableComponent>(Interactables, false);
		for (UInteractableComponent* Interactable : Interactables)
		{
			Interactable->RestrictInteractionToComponent(TriggeredComponent);
		}
	}
}

void UGrabBehavior::OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
								const FInputActionValue& Value)
{
	if (MyPhysicsComponent)
	{
		MyPhysicsComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		MyPhysicsComponent->SetSimulatePhysics(true);
	}
	else
	{
		GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	if (bBlockOtherInteractionsWhileGrabbed)
	{
		TArray<UInteractableComponent*> Interactables;
		GetOwner()->GetComponents<UInteractableComponent>(Interactables, false);
		for (UInteractableComponent* Interactable : Interactables)
		{
			Interactable->ResetRestrictInteraction();
		}
	}
}
