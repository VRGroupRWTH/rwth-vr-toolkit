// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/GrabBehavior.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionEventType.h"
#include "Logging/StructuredLog.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"
#include "Serialization/JsonTypes.h"
#include "Utility/RWTHVRUtilities.h"

UGrabBehavior::UGrabBehavior()
{
	SetIsReplicatedByDefault(true);
	bExecuteOnServer = true;
	bExecuteOnAllClients = false;
}

void UGrabBehavior::BeginPlay()
{
	Super::BeginPlay();

	OnActionReplicationStartedOriginatorEvent.AddDynamic(this, &UGrabBehavior::ReplicationOriginaterClientCallback);
}

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
void UGrabBehavior::ReplicationOriginaterClientCallback(USceneComponent* TriggerComponent,
														const EInteractionEventType EventType,
														const FInputActionValue& Value)
{
	const USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggerComponent->GetAttachParent());
	HandleCollisionHandlingMovement(CurrentAttachParent, EventType);
}

void UGrabBehavior::HandleCollisionHandlingMovement(const USceneComponent* CurrentAttachParent, const EInteractionEventType EventType)
{
	auto CHM = CurrentAttachParent->GetOwner()->GetComponentByClass<UCollisionHandlingMovement>();
	if(!CHM)
		return;
	
	if (EventType == EInteractionEventType::InteractionStart)
	{
		// Add to ignore list for collision handling movement, if it exists
		if (bIgnoreGrabbedActorInCollisionMovement)
		{
			bWasAddedToIgnore = CHM->AddActorToIgnore(GetOwner());
		}
	}
	else
	{
		// If our attach parent has a collision handling component, remove
		if (bWasAddedToIgnore)
		{
			CHM->RemoveActorFromIgnore(GetOwner());
		}
	}

}
void UGrabBehavior::OnActionEvent(USceneComponent* TriggerComponent, const EInteractionEventType EventType,
								  const FInputActionValue& Value)
{
	if (EventType == EInteractionEventType::InteractionStart)
	{
		StartGrab(TriggerComponent);
	}
	else
	{
		EndGrab(TriggerComponent);
	}
}

bool UGrabBehavior::TryRelease()
{
	if (!bObjectGrabbed)
	{
		UE_LOGFMT(Toolkit, Display, "UGrabBehavior::TryRelease: bObjectGrabbed was false!");
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
void UGrabBehavior::StartGrab(USceneComponent* TriggerComponent)
{
	if (bObjectGrabbed)
	{
		return;
	}

	USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggerComponent->GetAttachParent());
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
			Interactable->RestrictInteractionToComponent(TriggerComponent);
		}
	}

	OnGrabStartEvent.Broadcast(CurrentAttachParent, MyPhysicsComponent);

	// Add to ignore list for collision handling movement, if it exists
	HandleCollisionHandlingMovement(CurrentAttachParent, InteractionStart);
}

void UGrabBehavior::EndGrab(USceneComponent* TriggerComponent)
{
	USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggerComponent->GetAttachParent());

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

	// If our attach parent has a collision handling component, remove
	HandleCollisionHandlingMovement(CurrentAttachParent, InteractionEnd);
}
