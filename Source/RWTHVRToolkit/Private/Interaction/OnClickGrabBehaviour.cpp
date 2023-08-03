#include "Interaction/OnClickGrabBehaviour.h"
#include "Kismet/GameplayStatics.h"

UOnClickGrabBehaviour::UOnClickGrabBehaviour()
{

}

UPrimitiveComponent* UOnClickGrabBehaviour::GetFirstComponentSimulatingPhysics(const AActor* TargetActor)
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
UPrimitiveComponent* UOnClickGrabBehaviour::GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp)
{
	if (Cast<UPrimitiveComponent>(Comp->GetAttachParent()) && Comp->GetAttachParent()->IsSimulatingPhysics()) {
		return GetHighestParentSimulatingPhysics(Cast<UPrimitiveComponent>(Comp->GetAttachParent()));
	}
	else {
		return Comp;
	}
}

void UOnClickGrabBehaviour::OnClickStart(const UIntenSelectComponent* IntenSelect, const FVector& Point)
{
	const auto Player = UGameplayStatics::GetPlayerPawn(this, 0);
	
	USceneComponent* RightHand = Cast<USceneComponent>(Player->GetDefaultSubobjectByName("Right Hand"));
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);

	MyPhysicsComponent = GetFirstComponentSimulatingPhysics(GetOwner());

	if (MyPhysicsComponent) {
		MyPhysicsComponent->SetSimulatePhysics(false);
		MyPhysicsComponent->AttachToComponent(RightHand, Rules);
	}
	else {
		GetOwner()->GetRootComponent()->AttachToComponent(RightHand, Rules);
	}

}

void UOnClickGrabBehaviour::OnClickEnd(const UIntenSelectComponent* IntenSelect)
{
	if(MyPhysicsComponent)
	{
		MyPhysicsComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		MyPhysicsComponent->SetSimulatePhysics(true);
	}else
	{
		GetOwner()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}