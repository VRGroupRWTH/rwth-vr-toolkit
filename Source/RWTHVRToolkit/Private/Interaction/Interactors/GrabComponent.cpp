// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/GrabComponent.h"

#include "EnhancedInputComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionBitSet.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<UInteractableComponent*> CurrentGrabCompsInRange;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	TArray<FHitResult> OutHits;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);

	auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetAttachParent()->GetComponentLocation(),
										   GetAttachParent()->GetComponentLocation(), GrabSphereRadius, TraceType, true,
										   ActorsToIgnore, DebugTrace, OutHits, true, FColor::Green);

	for (FHitResult Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UInteractableComponent* Grabbable = SearchForInteractable(HitActor);
			if (Grabbable && Grabbable->HasInteractionTypeFlag(EInteractorType::Grab) && Grabbable->IsInteractable)
			{
				Grabbable->HitResult = Hit;
				CurrentGrabCompsInRange.AddUnique(Grabbable);
			}
		}
	}

	CurrentGrabBehavioursInRange = CurrentGrabCompsInRange;

	// Call hover start events on all components that were not in range before
	for (UInteractableComponent* CurrentGrabbable : CurrentGrabCompsInRange)
	{
		if (!PreviousGrabBehavioursInRange.Contains(CurrentGrabbable))
		{
			PreviousGrabBehavioursInRange.AddUnique(CurrentGrabbable);
			CurrentGrabbable->HandleOnHoverStartEvents(this, EInteractorType::Grab);
		}
	}

	TArray<UInteractableComponent*> ComponentsToRemove;

	// Call hover end events on all components that were previously in range, but not anymore
	for (UInteractableComponent* PrevGrabbale : PreviousGrabBehavioursInRange)
	{
		if (!CurrentGrabCompsInRange.Contains(PrevGrabbale))
		{
			ComponentsToRemove.AddUnique(PrevGrabbale);
			PrevGrabbale->HandleOnHoverEndEvents(this, EInteractorType::Grab);
		}
	}

	for (UInteractableComponent* CompToRemove : ComponentsToRemove)
	{
		PreviousGrabBehavioursInRange.Remove(CompToRemove);
	}
}

void UGrabComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (EI == nullptr)
		return;

	EI->BindAction(GrabInputAction, ETriggerEvent::Started, this, &UGrabComponent::OnBeginGrab);
	EI->BindAction(GrabInputAction, ETriggerEvent::Completed, this, &UGrabComponent::OnEndGrab);
}

void UGrabComponent::OnBeginGrab(const FInputActionValue& Value)
{
	const FVector GrabLocation = GetAttachParent()->GetComponentLocation();

	if (CurrentGrabBehavioursInRange.IsEmpty())
		return;

	if (bOnlyGrabClosestActor)
	{
		auto MinElement = *Algo::MinElementBy(
			CurrentGrabBehavioursInRange,
			[&](auto Element) { return FVector(Element->GetOwner()->GetActorLocation() - GrabLocation).Size(); });
		MinElement->HandleOnActionStartEvents(this, GrabInputAction, Value, EInteractorType::Grab);
		CurrentlyGrabbedComponents = {MinElement};
	}
	else
	{
		CurrentlyGrabbedComponents.Reserve(CurrentlyGrabbedComponents.Num() + CurrentGrabBehavioursInRange.Num());
		for (UInteractableComponent* Grabbable : CurrentGrabBehavioursInRange)
		{
			Grabbable->HandleOnActionStartEvents(this, GrabInputAction, Value, EInteractorType::Grab);
			CurrentlyGrabbedComponents.Add(Grabbable);
		}
	}
}

void UGrabComponent::OnEndGrab(const FInputActionValue& Value)
{
	for (auto& Component : CurrentlyGrabbedComponents)
	{
		if (Component.IsValid())
		{
			Component->HandleOnActionEndEvents(this, GrabInputAction, Value, EInteractorType::Grab);
		}
	}
}

UInteractableComponent* UGrabComponent::SearchForInteractable(AActor* HitActor)
{
	UInteractableComponent* Grabbable = nullptr;
	if (!HitActor)
	{
		UE_LOGFMT(Toolkit, Warning, "UGrabComponent::SearchForInteractable: HitActor was nullptr, returning nullptr");
		return nullptr;
	}

	if (HitActor->IsChildActor())
	{
		// search for UInteractable upwards from hit geometry and return first one found
		Grabbable = HitActor->FindComponentByClass<UInteractableComponent>();
		// if Grabbable is not valid search at parent
		if (!Grabbable)
		{
			HitActor = HitActor->GetParentActor();
			if (HitActor)
			{
				bSearchAtParent = true;
				return SearchForInteractable(HitActor);
			}
		}
	}
	else if (!HitActor->IsChildActor())
	{
		Grabbable = HitActor->FindComponentByClass<UInteractableComponent>();
	}

	if (Grabbable)
	{
		// in the case, were we had to iterate up the hierarchy, check if we are allowed
		// to grab the parent via child geometry
		if (bSearchAtParent && !Grabbable->bAllowInteractionFromChildGeometry)
		{
			Grabbable = nullptr;
		}
	}
	bSearchAtParent = false;
	return Grabbable;
}
