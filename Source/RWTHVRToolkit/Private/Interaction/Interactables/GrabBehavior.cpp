#include "Interaction/Interactables/GrabBehavior.h"

#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionEventType.h"
#include "Logging/StructuredLog.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Serialization/JsonTypes.h"
#include "Utility/RWTHVRUtilities.h"

UGrabBehavior::UGrabBehavior()
{
	SetIsReplicatedByDefault(true);
	bExecuteOnServer = true;
	bExecuteOnAllClients = false;
	PrimaryComponentTick.bCanEverTick = true;
	UActorComponent::SetComponentTickEnabled(false);
	// SetTickGroup(TG_PrePhysics);
}

void UGrabBehavior::BeginPlay()
{
	Super::BeginPlay();

	OnActionReplicationStartedOriginatorEvent.AddDynamic(this, &UGrabBehavior::ReplicationOriginaterClientCallback);
}

void UGrabBehavior::TickComponent(float DeltaTime, enum ELevelTick TickType,
								  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bObjectGrabbed && bUsePhysicsGrab)
	{
		// CurrentlyActiveInteractorComponent->GetComponentTransform().TransformVector(HandleOffset)
		//   No rotation for now
		for (auto PhysicsHandleData : ActivePhysicsHandleComponents)
		{
			const FVector NewPosition =
				PhysicsHandleData.Value.Key->GetComponentLocation() + PhysicsHandleData.Value.Value;
			PhysicsHandleData.Key->SetTargetLocation(NewPosition);
		}
	}
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

void UGrabBehavior::HandleCollisionHandlingMovement(const USceneComponent* CurrentAttachParent,
													const EInteractionEventType EventType)
{
	auto CHM = CurrentAttachParent->GetOwner()->GetComponentByClass<UCollisionHandlingMovement>();
	if (!CHM)
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

bool UGrabBehavior::TryRelease(USceneComponent* TriggerComponent)
{
	if (!bObjectGrabbed)
	{
		UE_LOGFMT(Toolkit, Display, "UGrabBehavior::TryRelease: bObjectGrabbed was false!");
		return false;
	}
	if (bUsePhysicsGrab)
	{
		auto PhysicsHandle = TriggerComponent->GetOwner()->GetComponentByClass<UPhysicsHandleComponent>();
		if (!PhysicsHandle)
			return false;

		ActivePhysicsHandleComponents.Remove(PhysicsHandle);
		PhysicsHandle->ReleaseComponent();

		if (ActivePhysicsHandleComponents.IsEmpty())
		{
			MyPhysicsComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, PrevCollisionResponse);
			SetComponentTickEnabled(false);
			bObjectGrabbed = false;
		}
	}
	else
	{
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
	}
	return true;
}

void UGrabBehavior::StartGrab(USceneComponent* TriggerComponent)
{
	if (bObjectGrabbed && !bUsePhysicsGrab)
	{
		return;
	}

	MyPhysicsComponent = GetFirstComponentSimulatingPhysics(GetOwner());

	if (bUsePhysicsGrab)
	{
		// the triggering actor (pawn) should have a physics handle component:
		AActor* GrabbingActor = TriggerComponent->GetOwner();
		auto PhysicsHandleComponent = GrabbingActor->GetComponentByClass<UPhysicsHandleComponent>();
		if (PhysicsHandleComponent && MyPhysicsComponent)
		{

			FVector ClosestPoint;
			if (MyPhysicsComponent->GetClosestPointOnCollision(TriggerComponent->GetComponentLocation(),
															   ClosestPoint) <= 0)
				ClosestPoint = TriggerComponent->GetComponentLocation();
			auto HandleOffset = ClosestPoint - TriggerComponent->GetComponentLocation();
			PhysicsHandleComponent->GrabComponentAtLocation(MyPhysicsComponent, NAME_None, ClosestPoint);
			PrevCollisionResponse = MyPhysicsComponent->GetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn);
			MyPhysicsComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
			bObjectGrabbed = true;
			SetComponentTickEnabled(true);
			ActivePhysicsHandleComponents.Add(PhysicsHandleComponent, {TriggerComponent, HandleOffset});
		}
		else
		{
			UE_LOGFMT(Toolkit, Warning,
					  "Physics-based grab requires the object to be grabbed ({Me})to have a Primitive Component, as "
					  "well as a PhysicsHandle on the interacting comp {C}!",
					  this->GetName(), TriggerComponent->GetName());
		}
	}
	else
	{
		USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggerComponent->GetAttachParent());
		const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

		if (MyPhysicsComponent)
		{
			bWasSimulatingPhysics = MyPhysicsComponent->IsSimulatingPhysics();
			MyPhysicsComponent->SetSimulatePhysics(false);
			bObjectGrabbed = MyPhysicsComponent->AttachToComponent(CurrentAttachParent, Rules);
		}
		else
		{
			bObjectGrabbed = GetOwner()->GetRootComponent()->AttachToComponent(CurrentAttachParent, Rules);
		}
	}

	if (!bObjectGrabbed)
	{
		UE_LOGFMT(Toolkit, Warning, "Grab failed! Cannot attach grabbed component to attach parent ({Parent})",
				  TriggerComponent->GetName());
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

	OnGrabStartEvent.Broadcast(TriggerComponent, MyPhysicsComponent);

	// Add to ignore list for collision handling movement, if it exists
	HandleCollisionHandlingMovement(TriggerComponent, InteractionStart);
}

void UGrabBehavior::EndGrab(USceneComponent* TriggerComponent)
{
	USceneComponent* CurrentAttachParent = Cast<USceneComponent>(TriggerComponent->GetAttachParent());

	// We try to release the attached component. If it is not succesful we log and return. Otherwise, we continue.
	if (!TryRelease(TriggerComponent))
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
