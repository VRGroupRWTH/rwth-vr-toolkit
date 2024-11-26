// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/RaycastInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Interaction/Interactables/InteractionEventType.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
URaycastInteractionComponent::URaycastInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void URaycastInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
												 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UInteractableComponent* NewInteractableComponent = nullptr;

	FHitResult Hit;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);
	const FVector TraceStart = GetAttachParent()->GetComponentLocation();
	const FVector TraceEnd =
		GetAttachParent()->GetComponentLocation() + TraceLength * GetAttachParent()->GetForwardVector();

	const auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, ActorsToIgnore, DebugTrace,
										  Hit, true, FColor::Green);

	AActor* HitActor = Hit.GetActor();
	if (HitActor)
	{
		UInteractableComponent* InteractableComponent = HitActor->FindComponentByClass<UInteractableComponent>();
		if (InteractableComponent && InteractableComponent->IsInteractable)
		{
			NewInteractableComponent = InteractableComponent;
			InteractableComponent->HitResult = Hit;
		}
	}

	CurrentInteractable = NewInteractableComponent;

	if (CurrentInteractable != PreviousInteractable)
	{
		if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
			CurrentInteractable->HandleOnHoverEvents(this, EInteractorType::Raycast,
													 EInteractionEventType::InteractionStart);
		if (PreviousInteractable && PreviousInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
			PreviousInteractable->HandleOnHoverEvents(this, EInteractorType::Raycast,
													  EInteractionEventType::InteractionEnd);
	}

	PreviousInteractable = CurrentInteractable;
}

void URaycastInteractionComponent::OnBeginInteractionInputAction(const FInputActionValue& Value)
{
	if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
		CurrentInteractable->HandleOnActionEvents(this, EInteractorType::Raycast,
												  EInteractionEventType::InteractionStart, Value);
}

void URaycastInteractionComponent::OnEndInteractionInputAction(const FInputActionValue& Value)
{
	if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
		CurrentInteractable->HandleOnActionEvents(this, EInteractorType::Raycast, EInteractionEventType::InteractionEnd,
												  Value);
}

void URaycastInteractionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (!EI)
		return;

	EI->BindAction(InteractionInputAction, ETriggerEvent::Started, this,
				   &URaycastInteractionComponent::OnBeginInteractionInputAction);
	EI->BindAction(InteractionInputAction, ETriggerEvent::Completed, this,
				   &URaycastInteractionComponent::OnEndInteractionInputAction);
}
