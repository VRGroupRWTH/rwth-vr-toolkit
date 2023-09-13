// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/OnClickGrabBehavior.h"

#include "Interaction/GrabbableComponent.h"
#include "Interaction/InteractableBase.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonTypes.h"

// Sets default values for this component's properties
UOnClickGrabBehavior::UOnClickGrabBehavior()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOnClickGrabBehavior::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOnClickGrabBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UPrimitiveComponent* UOnClickGrabBehavior::GetFirstComponentSimulatingPhysics(const AActor* TargetActor)
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	// find any component that simulates physics, then traverse the hierarchy
	for (UPrimitiveComponent* const& Component : PrimitiveComponents) {
		if (Component->IsSimulatingPhysics()) {
			return GetHighestParentSimulatingPhysics(Component);
		}
	}
	return nullptr;
}

// recursively goes up the hierarchy and returns the highest parent simulating physics
UPrimitiveComponent* UOnClickGrabBehavior::GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp)
{
	if (Cast<UPrimitiveComponent>(Comp->GetAttachParent()) && Comp->GetAttachParent()->IsSimulatingPhysics()) {
		return GetHighestParentSimulatingPhysics(Cast<UPrimitiveComponent>(Comp->GetAttachParent()));
	}
	else {
		return Comp;
	}
}

void UOnClickGrabBehavior::OnClickStart(USceneComponent* TriggeredComponent, const FInputActionValue& Value)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	USceneComponent* Hand = Cast<USceneComponent>(TriggeredComponent->GetAttachParent());
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

	MyPhysicsComponent = GetFirstComponentSimulatingPhysics(GetOwner());

	if (MyPhysicsComponent) {
		MyPhysicsComponent->SetSimulatePhysics(false);
		MyPhysicsComponent->AttachToComponent(Hand, Rules);
	}
	else {
		GetOwner()->GetRootComponent()->AttachToComponent(Hand, Rules);
	}

	
	if(bBlockOtherInteractionsWhileGrabbed)
	{
		TArray<UInteractableBase*> Interactables; 
		GetOwner()->GetComponents<UInteractableBase>(Interactables,false);
		for (UInteractableBase* Interactable : Interactables)
		{
			Interactable->RestrictInteractionToComponent(TriggeredComponent);
		}
	}
}

void UOnClickGrabBehavior::OnClickEnd(USceneComponent* TriggeredComponent, const FInputActionValue& Value)
{
	if(MyPhysicsComponent)
	{
		MyPhysicsComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		MyPhysicsComponent->SetSimulatePhysics(true);
	}else
	{
		GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	if(bBlockOtherInteractionsWhileGrabbed)
	{
		TArray<UInteractableBase*> Interactables; 
		GetOwner()->GetComponents<UInteractableBase>(Interactables,false);
		for (UInteractableBase* Interactable : Interactables)
		{
			Interactable->ResetRestrictInteraction();
		}
	}
}


