// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/NoAttachGrabBehavior.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Logging/StructuredLog.h"
#include "Serialization/JsonTypes.h"
#include "Utility/RWTHVRUtilities.h"

UNoAttachGrabBehavior::UNoAttachGrabBehavior()
{

	ProxyComponent = CreateDefaultSubobject<USceneComponent>("ProxyComponent");
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void UNoAttachGrabBehavior::OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
										  const FInputActionValue& Value)
{
	if (bObjectGrabbed)
	{
		return;
	}

	CurrentAttachParent = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

	FVector ComponentLocation;
	ParentActor = GetOwner();
	if (bAffectsParentActor)
	{
		while (ParentActor->GetAttachParentActor() != NULL)
		{
			ParentActor = ParentActor->GetAttachParentActor();
		}
		ComponentLocation = ParentActor->GetRootComponent()->GetComponentLocation();
	}
	else
	{
		ComponentLocation = GetComponentLocation();
	}

	ProxyComponent->SetWorldLocation(ComponentLocation);
	bObjectGrabbed = ProxyComponent->AttachToComponent(CurrentAttachParent, Rules);

	SetComponentTickEnabled(true);

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

	OnGrabStartEvent.Broadcast(CurrentAttachParent, NULL);
}

void UNoAttachGrabBehavior::OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
										const FInputActionValue& Value)
{

	CurrentAttachParent = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());

	// We try to release the attached component. If it is not succesful we log and return. Otherwise, we continue.
	if (!TryRelease())
	{
		UE_LOGFMT(Toolkit, Display,
				  "UNoAttachGrabBehavior::OnActionEnd: TryRelease failed to release with AttachParent {Parent}",
				  CurrentAttachParent->GetName());
		return;
	}

	OnGrabEndEvent.Broadcast(CurrentAttachParent, NULL);

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

	SetComponentTickEnabled(false);
}
void UNoAttachGrabBehavior::TickComponent(float DeltaTime, ELevelTick TickType,
										  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bObjectGrabbed)
	{
		return;
	}

	USceneComponent* AffectedComponent;

	if (bAffectsParentActor)
	{
		AffectedComponent = ParentActor->GetRootComponent();
	}
	else
	{
		AffectedComponent = GetOwner()->GetRootComponent();
	}

	if (bSyncLocation)
	{
		AffectedComponent->SetWorldLocation(ProxyComponent->GetComponentLocation());
	}

	if (bSyncRotation)
	{
		AffectedComponent->SetWorldRotation(ProxyComponent->GetComponentRotation());
	}
}

bool UNoAttachGrabBehavior::TryRelease()
{
	if (!bObjectGrabbed)
	{
		return false;
	}

	ProxyComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	bObjectGrabbed = false;
	return true;
}
