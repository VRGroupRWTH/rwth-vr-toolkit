// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/BasicVRInteractionComponent.h"


#include "Interaction/Clickable.h"
#include "Interaction/Grabable.h"
#include "Interaction/Targetable.h"
#include "Interaction/GrabbingBehaviorComponent.h"
#include "Misc/Optional.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UBasicVRInteractionComponent::UBasicVRInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UBasicVRInteractionComponent::BeginInteraction()
{
	if(!InteractionRayEmitter) return;
	
	// start and end point for raytracing
	const FTwoVectors StartEnd = GetHandRay(MaxClickDistance);
	TOptional<FHitResult> Hit = RaytraceForFirstHit(StartEnd);
	if (!Hit.IsSet())
		return;

	AActor* HitActor = Hit->GetActor();
	
	if (HitActor->Implements<UGrabable>() && Hit->Distance < MaxGrabDistance)
	{
		// call grabable actors function so he reacts to our grab
		IGrabable::Execute_OnBeginGrab(HitActor);
		
		// save it for later, is needed every tick
		Behavior = HitActor->FindComponentByClass<UGrabbingBehaviorComponent>();
		if ( Behavior == nullptr)
			HandlePhysicsAndAttachActor(HitActor);

		// we save the grabbedActor in a general form to access all of AActors functions easily later
		GrabbedActor = HitActor;
	}
	else if (HitActor->Implements<UClickable>() && Hit->Distance < MaxClickDistance)
	{
		IClickable::Execute_OnClick(HitActor, Hit->Location);
	}
}

void UBasicVRInteractionComponent::EndInteraction()
{
	if(!InteractionRayEmitter) return;
	
	// if we didnt grab anyone there is no need to release
	if (GrabbedActor == nullptr)
		return;

	// let the grabbed object react to release
	IGrabable::Execute_OnEndGrab(GrabbedActor);

	// Detach the Actor
	if (GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>() == nullptr)
	{
		if (ComponentSimulatingPhysics) {
			ComponentSimulatingPhysics->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			ComponentSimulatingPhysics->SetSimulatePhysics(true);
		}
		else {
			GrabbedActor->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}
	}

	// forget about the actor
	GrabbedActor = nullptr;
	ComponentSimulatingPhysics = nullptr;
	Behavior = nullptr;
}

// Called every frame
void UBasicVRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InteractionRayEmitter) return;
	
	// if our Grabable Actor is not constrained we need to calculate the position dynamically
	if (Behavior != nullptr)
	{	
		// specifies the hand in space
		const FVector HandPos = InteractionRayEmitter->GetComponentLocation();
		const FQuat HandQuat = InteractionRayEmitter->GetComponentQuat();

		Behavior->HandleNewPositionAndDirection(HandPos, HandQuat); 
	}

	// only raytrace for targetable objects if bool user wants to enable this feature
	if (!bCanRaytraceEveryTick)
		return;

	const FTwoVectors StartEnd = GetHandRay(MaxClickDistance);
	TOptional<FHitResult> Hit = RaytraceForFirstHit(StartEnd);
	if (!Hit.IsSet())
		return;
	AActor* HitActor = Hit->GetActor();

	// for now uses the same distance as clicking
	if (HitActor->Implements<UTargetable>() && Hit->Distance < MaxClickDistance)
	{
		ITargetable::Execute_OnTargeted(HitActor, Hit->Location);
	}
}

void UBasicVRInteractionComponent::Initialize(USceneComponent* RayEmitter, float InMaxGrabDistance, float InMaxClickDistance)
{
	if(InteractionRayEmitter) return; /* Return if already initialized */

	InteractionRayEmitter = RayEmitter;
	MaxGrabDistance = InMaxGrabDistance;
	MaxClickDistance = InMaxClickDistance;
}

void UBasicVRInteractionComponent::HandlePhysicsAndAttachActor(AActor* HitActor)
{
	UPrimitiveComponent* PhysicsSimulatingComp = GetFirstComponentSimulatingPhysics(HitActor);
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, false);
	
	if (PhysicsSimulatingComp) {
		PhysicsSimulatingComp->SetSimulatePhysics(false);
		PhysicsSimulatingComp->AttachToComponent(InteractionRayEmitter, Rules);	
		ComponentSimulatingPhysics = PhysicsSimulatingComp;
	}
	else {
		HitActor->GetRootComponent()->AttachToComponent(InteractionRayEmitter, Rules);
	}
}

FTwoVectors UBasicVRInteractionComponent::GetHandRay(const float Length) const
{
	const FVector Start = InteractionRayEmitter->GetComponentLocation();
	const FVector Direction = InteractionRayEmitter->GetForwardVector();
	const FVector End = Start + Length * Direction;

	return FTwoVectors(Start, End);
}

TOptional<FHitResult> UBasicVRInteractionComponent::RaytraceForFirstHit(const FTwoVectors& Ray) const
{
	const FVector Start = Ray.v1;
	const FVector End   = Ray.v2;	
	
	// will be filled by the Line Trace Function
	FHitResult Hit;

	const FCollisionObjectQueryParams Params;	
	FCollisionQueryParams Params2; 
	Params2.AddIgnoredActor(GetOwner()->GetUniqueID()); // prevents actor hitting itself
	if (GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, Params, Params2))
		return {Hit};
	else
		return {};
}

UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor)
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
UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp)
{	
	if (Cast<UPrimitiveComponent>(Comp->GetAttachParent()) && Comp->GetAttachParent()->IsSimulatingPhysics()) {
		return GetHighestParentSimulatingPhysics(Cast<UPrimitiveComponent>(Comp->GetAttachParent()));
	}
	else {
		return Comp;
	}
}
