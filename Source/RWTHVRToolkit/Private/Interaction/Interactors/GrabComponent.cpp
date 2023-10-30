// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/GrabComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/Interactees/GrabbableComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	TArray<UGrabbableComponent*> CurrentGrabCompsInRange;

	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> OutHits;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);

	auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetAttachParent()->GetComponentLocation(),
	                                       GetAttachParent()->GetComponentLocation(), GrabSphereRadius, TraceType, true,
	                                       ActorsToIgnore, DebugTrace,
	                                       OutHits, true, FColor::Green);

	for (FHitResult Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UGrabbableComponent* Grabbable = HitActor->FindComponentByClass<UGrabbableComponent>();
			if (Grabbable && Grabbable->IsInteractable)
			{
				Grabbable->HitResult = Hit;
				CurrentGrabCompsInRange.Add(Grabbable);
			}
		}
	}

	CurrentGrabbableInRange = CurrentGrabCompsInRange;

	// Call hover start events on all components that were not in range before
	for (UGrabbableComponent* CurrentGrabbale : CurrentGrabCompsInRange)
	{
		if (!PreviousGrabbablesInRange.Contains(CurrentGrabbale))
		{
			PreviousGrabbablesInRange.Add(CurrentGrabbale);
			CurrentGrabbale->HandleOnHoverStartEvents(this);
		}
	}

	TArray<UGrabbableComponent*> ComponentsToRemove;

	// Call hover end events on all components that were previously in range, but not anymore
	for (UGrabbableComponent* PrevGrabbale : PreviousGrabbablesInRange)
	{
		if (!CurrentGrabCompsInRange.Contains(PrevGrabbale))
		{
			ComponentsToRemove.Add(PrevGrabbale);
			PrevGrabbale->HandleOnHoverEndEvents(this);
		}
	}

	for (UGrabbableComponent* CompToRemove : ComponentsToRemove)
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
	for (UGrabbableComponent* Grabbale : CurrentGrabbableInRange)
	{
		Grabbale->HandleOnClickStartEvents(this, Value);
	}
}

void UGrabComponent::OnEndGrab(const FInputActionValue& Value)
{
	for (UGrabbableComponent* Grabbale : CurrentGrabbableInRange)
	{
		Grabbale->HandleOnClickEndEvents(this, Value);
	}
}
