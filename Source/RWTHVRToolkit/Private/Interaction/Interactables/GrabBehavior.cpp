// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/GrabBehavior.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Logging/StructuredLog.h"
#include "Serialization/JsonTypes.h"
#include "Utility/RWTHVRUtilities.h"

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

	return Comp;
}

void UGrabBehavior::OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
								  const FInputActionValue& Value)
{
	if (bObjectGrabbed)
	{
		return;
	}

	USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

	if (MyPhysicsComponent = GetFirstComponentSimulatingPhysics(GetOwner()); MyPhysicsComponent != nullptr)
	{
		bWasSimulatingPhysics = MyPhysicsComponent->IsSimulatingPhysics();
		MyPhysicsComponent->SetSimulatePhysics(false);
		bObjectGrabbed = MyPhysicsComponent->AttachToComponent(CurrentAttachParent, Rules);
	}
	else
	{
		bObjectGrabbed = GetOwner()->GetRootComponent()->AttachToComponent(CurrentAttachParent, Rules);
	}

	if (!bObjectGrabbed)
	{
		UE_LOGFMT(Toolkit, Warning, "Grab failed! Cannot attach grabbed component to attach parent ({Parent})",
				  CurrentAttachParent->GetName());
		return;
	}

	// If we want to restrict other interactions while this component is grabbed we add the component
	// that triggered the interaction to the whitelist of all interactables that are attached to the
	// affected actor
	if (bBlockOtherInteractionsWhileGrabbed)
	{
		TArray<UInteractableComponent*> Interactables;
		GetOwner()->GetComponents<UInteractableComponent>(Interactables, false);
		for (UInteractableComponent* Interactable : Interactables)
		{
			Interactable->RestrictInteractionToComponent(TriggeredComponent);
		}
	}

	OnGrabStartEvent.Broadcast(CurrentAttachParent, MyPhysicsComponent);
}

void UGrabBehavior::OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
								const FInputActionValue& Value)
{

	USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());

	// We try to release the attached component. If it is not succesful we log and return. Otherwise, we continue.
	if (!TryRelease())
	{
		UE_LOGFMT(Toolkit, Display,
				  "UGrabBehavior::OnActionEnd: TryRelease failed to release with AttachParent {Parent}",
				  CurrentAttachParent->GetName());
		return;
	}

	OnGrabEndEvent.Broadcast(CurrentAttachParent, MyPhysicsComponent);

	// Release the interation restriction on all component
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

bool UGrabBehavior::TryRelease()
{
	if (!bObjectGrabbed)
	{
		return false;
	}

	if (MyPhysicsComponent)
	{
		MyPhysicsComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		MyPhysicsComponent->SetSimulatePhysics(bWasSimulatingPhysics);
	}
	else
	{
		GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	bObjectGrabbed = false;
	return true;
}
