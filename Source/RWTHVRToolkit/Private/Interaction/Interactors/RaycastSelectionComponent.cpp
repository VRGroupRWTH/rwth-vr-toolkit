// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/RaycastSelectionComponent.h"

#include "EnhancedInputComponent.h"
#include "Interaction/Interactables/InteractableComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
URaycastSelectionComponent::URaycastSelectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void URaycastSelectionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
											   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UInteractableComponent* CurrentSelectable = nullptr;


	TArray<AActor*> ActorsToIgnore;
	FHitResult Hit;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);
	FVector TraceStart = GetAttachParent()->GetComponentLocation();
	FVector TraceEnd = GetAttachParent()->GetComponentLocation() + TraceLength * GetAttachParent()->GetForwardVector();

	auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, ActorsToIgnore, DebugTrace,
										  Hit, true, FColor::Green);

	AActor* HitActor = Hit.GetActor();
	if (HitActor)
	{
		UInteractableComponent* Selectable = HitActor->FindComponentByClass<UInteractableComponent>();
		if (Selectable && Selectable->IsInteractable)
		{
			CurrentSelectable = Selectable;
			Selectable->HitResult = Hit;
		}
	}

	CurrentInteractable = CurrentSelectable;

	if (CurrentInteractable != PreviousInteractable)
	{
		if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
			CurrentInteractable->HandleOnHoverStartEvents(this, EInteractorType::Raycast);
		if (PreviousInteractable && PreviousInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
			PreviousInteractable->HandleOnHoverEndEvents(this, EInteractorType::Raycast);
	}

	PreviousInteractable = CurrentInteractable;
}

void URaycastSelectionComponent::OnBeginSelect(const FInputActionValue& Value)
{
	if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
		CurrentInteractable->HandleOnActionStartEvents(this, RayCastSelectInputAction, Value, EInteractorType::Raycast);
}

void URaycastSelectionComponent::OnEndSelect(const FInputActionValue& Value)
{
	if (CurrentInteractable && CurrentInteractable->HasInteractionTypeFlag(EInteractorType::Raycast))
		CurrentInteractable->HandleOnActionEndEvents(this, RayCastSelectInputAction, Value, EInteractorType::Raycast);
}

void URaycastSelectionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (!EI)
		return;

	EI->BindAction(RayCastSelectInputAction, ETriggerEvent::Started, this, &URaycastSelectionComponent::OnBeginSelect);
	EI->BindAction(RayCastSelectInputAction, ETriggerEvent::Completed, this, &URaycastSelectionComponent::OnEndSelect);
}
