// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/GrabComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionBitSet.h"

#include "Kismet/GameplayStatics.h"

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

	CurrentGrabbablesInRange = CurrentGrabCompsInRange;

	// Call hover start events on all components that were not in range before
	for (UInteractableComponent* CurrentGrabbale : CurrentGrabCompsInRange)
	{
		if (!PreviousGrabbablesInRange.Contains(CurrentGrabbale))
		{
			PreviousGrabbablesInRange.AddUnique(CurrentGrabbale);
			CurrentGrabbale->HandleOnHoverStartEvents(this, EInteractorType::Grab);
		}
	}

	TArray<UInteractableComponent*> ComponentsToRemove;

	// Call hover end events on all components that were previously in range, but not anymore
	for (UInteractableComponent* PrevGrabbale : PreviousGrabbablesInRange)
	{
		if (!CurrentGrabCompsInRange.Contains(PrevGrabbale))
		{
			ComponentsToRemove.AddUnique(PrevGrabbale);
			PrevGrabbale->HandleOnHoverEndEvents(this, EInteractorType::Grab);
		}
	}

	for (UInteractableComponent* CompToRemove : ComponentsToRemove)
	{
		PreviousGrabbablesInRange.Remove(CompToRemove);
	}
}

void UGrabComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	auto* InputSubsystem = GetEnhancedInputLocalPlayerSubsystem(Pawn);
	if (!InputSubsystem)
		return;

	// add Input Mapping context
	InputSubsystem->AddMappingContext(IMCGrab, 0);

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (EI == nullptr)
		return;

	EI->BindAction(GrabInputAction, ETriggerEvent::Started, this, &UGrabComponent::OnBeginGrab);
	EI->BindAction(GrabInputAction, ETriggerEvent::Completed, this, &UGrabComponent::OnEndGrab);
}

void UGrabComponent::OnBeginGrab(const FInputActionValue& Value)
{
	UInteractableComponent* ClosestGrabbable;
	float DistanceToClosestGrabbable;
	float DistanceToCurrentGrabbable;
	const FVector GrabLocation = GetAttachParent()->GetComponentLocation();

	if (CurrentGrabbablesInRange.IsEmpty())
		return;

	ClosestGrabbable = CurrentGrabbablesInRange.Last();
	FVector ClosestGrabbableLocation = ClosestGrabbable->GetOwner()->GetActorLocation();
	DistanceToClosestGrabbable = FVector(ClosestGrabbableLocation - GrabLocation).Size();

	for (UInteractableComponent* Grabbable : CurrentGrabbablesInRange)
	{
		if (bOnlyGrabClosestActor)
		{
			// find closest grabbable
			FVector CurrentGrabbableLocation = Grabbable->GetOwner()->GetActorLocation();
			DistanceToCurrentGrabbable = FVector(CurrentGrabbableLocation - GrabLocation).Size();

			if (DistanceToCurrentGrabbable < DistanceToClosestGrabbable)
			{
				DistanceToClosestGrabbable = DistanceToCurrentGrabbable;
				ClosestGrabbable = Grabbable;
			}

		}
		else
		{
			Grabbable->HandleOnActionStartEvents(this, GrabInputAction, Value, EInteractorType::Grab);
		}
	}

	if (bOnlyGrabClosestActor)
	{
		ClosestGrabbable->HandleOnActionStartEvents(this, GrabInputAction, Value, EInteractorType::Grab);
	}
}

void UGrabComponent::OnEndGrab(const FInputActionValue& Value)
{
	for (UInteractableComponent* Grabbable : CurrentGrabbablesInRange)
	{
		Grabbable->HandleOnActionEndEvents(this, GrabInputAction, Value, EInteractorType::Grab);
	}
}

UInteractableComponent* UGrabComponent::SearchForInteractable(AActor* HitActor)
{
	UInteractableComponent* Grabbable = nullptr;
	if (!HitActor)
	{
		return Grabbable;
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
				return SearchForInteractable(HitActor);
			}
		}
	}
	else if (!HitActor->IsChildActor())
	{
		Grabbable = HitActor->FindComponentByClass<UInteractableComponent>();
	}

	return Grabbable;
}
