// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/GrabbingBehaviorComponent.h"

// Sets default values for this component's properties
UGrabbingBehaviorComponent::UGrabbingBehaviorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UPrimitiveComponent* UGrabbingBehaviorComponent::GetFirstComponentSimulatingPhysics(const AActor* TargetActor)
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	// find any component that simulates physics, then traverse the hierarchy
	for (const auto& Component : PrimitiveComponents) {
		if (Component->IsSimulatingPhysics()) {
			return GetHighestParentSimulatingPhysics(Component);
		}
	}
	return nullptr;
}

// recursively goes up the hierarchy and returns the highest parent simulating physics
UPrimitiveComponent* UGrabbingBehaviorComponent::GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp)
{
	if (Cast<UPrimitiveComponent>(Comp->GetAttachParent()) && Comp->GetAttachParent()->IsSimulatingPhysics()) {
		return GetHighestParentSimulatingPhysics(Cast<UPrimitiveComponent>(Comp->GetAttachParent()));
	}
	else {
		return Comp;
	}
}

void UGrabbingBehaviorComponent::HandleNewPositionAndDirection(FVector Position, FQuat Orientation)
{

}

void UGrabbingBehaviorComponent::HandleBeginGrab(AActor* GrabbedBy)
{
	USceneComponent* RightHand = Cast<USceneComponent>(GrabbedBy->GetDefaultSubobjectByName("Right Hand"));
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);
	
	//GetOwner()->GetRootComponent()->AttachToComponent(RightHand, Rules);
	GetOwner()->GetRootComponent()->AttachToComponent(RightHand, Rules);

	OnBeginGrab.Broadcast(GrabbedBy);
}

void UGrabbingBehaviorComponent::HandleEndGrab()
{
	//GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	OnEndGrab.Broadcast();
}


void UGrabbingBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	MyPhysicsComponent = GetFirstComponentSimulatingPhysics(this->GetOwner());
}

void UGrabbingBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



