// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/DirectInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionBitSet.h"
#include "Interaction/Interactables/InteractionEventType.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

// Sets default values for this component's properties
UDirectInteractionComponent::UDirectInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

void UDirectInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
												FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<UInteractableComponent*> CurrentInteractableCompsInRange;

	TArray<FHitResult> OutHits;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);

	const auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetAttachParent()->GetComponentLocation(),
										   GetAttachParent()->GetComponentLocation(), InteractionSphereRadius,
										   TraceType, true, ActorsToIgnore, DebugTrace, OutHits, true, FColor::Green);

	for (FHitResult Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UInteractableComponent* InteractableComp = SearchForInteractable(HitActor);
			if (InteractableComp && InteractableComp->HasInteractionTypeFlag(EInteractorType::Direct) &&
				InteractableComp->IsInteractable)
			{
				InteractableComp->HitResult = Hit;
				CurrentInteractableCompsInRange.AddUnique(InteractableComp);
			}
		}
	}

	CurrentInteractableComponentsInRange = CurrentInteractableCompsInRange;

	// Call hover start events on all components that were not in range before
	for (UInteractableComponent* CurrentInteractableComp : CurrentInteractableCompsInRange)
	{
		if (!PreviousInteractableComponentsInRange.Contains(CurrentInteractableComp))
		{
			PreviousInteractableComponentsInRange.AddUnique(CurrentInteractableComp);
			CurrentInteractableComp->HandleOnHoverEvents(this, EInteractorType::Direct,
														 EInteractionEventType::InteractionStart);
		}
	}

	TArray<UInteractableComponent*> ComponentsToRemove;

	// Call hover end events on all components that were previously in range, but not anymore
	for (UInteractableComponent* PrevInteractableComp : PreviousInteractableComponentsInRange)
	{
		if (!CurrentInteractableCompsInRange.Contains(PrevInteractableComp))
		{
			ComponentsToRemove.AddUnique(PrevInteractableComp);
			PrevInteractableComp->HandleOnHoverEvents(this, EInteractorType::Direct,
													  EInteractionEventType::InteractionEnd);
		}
	}

	for (UInteractableComponent* CompToRemove : ComponentsToRemove)
	{
		PreviousInteractableComponentsInRange.Remove(CompToRemove);
	}
}

void UDirectInteractionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	// Probably not the best place to add this.
	ActorsToIgnore.AddUnique(GetOwner());

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (EI == nullptr)
		return;

	EI->BindAction(InteractionInputAction, ETriggerEvent::Started, this,
				   &UDirectInteractionComponent::OnBeginInteractionInputAction);
	EI->BindAction(InteractionInputAction, ETriggerEvent::Completed, this,
				   &UDirectInteractionComponent::OnEndInteractionInputAction);
}

void UDirectInteractionComponent::OnBeginInteractionInputAction(const FInputActionValue& Value)
{
	const FVector InteractionLocation = GetAttachParent()->GetComponentLocation();

	if (CurrentInteractableComponentsInRange.IsEmpty())
		return;

	if (bOnlyInteractWithClosestActor)
	{
		auto MinElement = *Algo::MinElementBy(
			CurrentInteractableComponentsInRange,
			[&](auto Element)
			{ return FVector(Element->GetOwner()->GetActorLocation() - InteractionLocation).Size(); });
		MinElement->HandleOnActionEvents(this, EInteractorType::Direct, EInteractionEventType::InteractionStart, Value);
		CurrentlyInteractedComponents = {MinElement};
	}
	else
	{
		CurrentlyInteractedComponents.Reserve(CurrentlyInteractedComponents.Num() +
											  CurrentInteractableComponentsInRange.Num());
		for (UInteractableComponent* InteractableComp : CurrentInteractableComponentsInRange)
		{
			InteractableComp->HandleOnActionEvents(this, EInteractorType::Direct,
												   EInteractionEventType::InteractionStart, Value);
			CurrentlyInteractedComponents.Add(InteractableComp);
		}
	}
}

void UDirectInteractionComponent::OnEndInteractionInputAction(const FInputActionValue& Value)
{
	for (auto& Component : CurrentlyInteractedComponents)
	{
		if (Component.IsValid())
		{
			Component->HandleOnActionEvents(this, EInteractorType::Direct, EInteractionEventType::InteractionStart,
											Value);
		}
	}
}

UInteractableComponent* UDirectInteractionComponent::SearchForInteractable(AActor* HitActor)
{
	UInteractableComponent* InteractableComponent = nullptr;
	if (!HitActor)
	{
		UE_LOGFMT(Toolkit, Warning,
				  "UDirectInteractionComponent::SearchForInteractable: HitActor was nullptr, returning nullptr");
		return nullptr;
	}

	if (HitActor->IsChildActor())
	{
		// search for UInteractable upwards from hit geometry and return first one found
		InteractableComponent = HitActor->FindComponentByClass<UInteractableComponent>();
		// if InteractableComponen is not valid search at parent
		if (!InteractableComponent)
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
		InteractableComponent = HitActor->FindComponentByClass<UInteractableComponent>();
	}

	if (InteractableComponent)
	{
		// in the case, were we had to iterate up the hierarchy, check if we are allowed
		// to interact with the parent via child geometry
		if (bSearchAtParent && !InteractableComponent->bAllowInteractionFromChildGeometry)
		{
			InteractableComponent = nullptr;
		}
	}
	bSearchAtParent = false;
	return InteractableComponent;
}
