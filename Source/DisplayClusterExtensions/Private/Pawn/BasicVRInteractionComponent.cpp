// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/BasicVRInteractionComponent.h"


#include "Interaction/Clickable.h"
#include "Interaction/Grabable.h"
#include "Interaction/GrabbingBehaviorComponent.h"

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
	const FVector Start = StartEnd.v1;
	const FVector End   = StartEnd.v2;	

	// will be filled by the Line Trace Function
	FHitResult Hit;

	//if hit was not found return  
	const FCollisionObjectQueryParams Params;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, Params))
		return;

	AActor* HitActor = Hit.GetActor();
	
	// try to cast HitActor into a Grabable if not succeeded will become a nullptr
	IGrabable*  GrabableActor  = Cast<IGrabable>(HitActor);
	IClickable* ClickableActor = Cast<IClickable>(HitActor);

	if (GrabableActor != nullptr && Hit.Distance < MaxGrabDistance)
	{
		// call grabable actors function so he reacts to our grab
		GrabableActor->OnGrabbed_Implementation();
		
		// save it for later, is needed every tick
		Behavior = HitActor->FindComponentByClass<UGrabbingBehaviorComponent>();
		if ( Behavior == nullptr)
			HandlePhysicsAndAttachActor(HitActor);

		// we save the grabbedActor in a general form to access all of AActors functions easily later
		GrabbedActor = HitActor;
	}
	else if (ClickableActor != nullptr && Hit.Distance < MaxClickDistance)
	{
		ClickableActor->OnClicked_Implementation(Hit.Location);
	}
}


void UBasicVRInteractionComponent::EndInteraction()
{
	if(!InteractionRayEmitter) return;
	
	// if we didnt grab anyone there is no need to release
	if (GrabbedActor == nullptr)
		return;

	// let the grabbed object react to release
	Cast<IGrabable>(GrabbedActor)->OnReleased_Implementation();

	// Detach the Actor
	if (GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>() == nullptr)
	{
		GrabbedActor->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GrabbedActor->FindComponentByClass<UPrimitiveComponent>()->SetSimulatePhysics(bDidSimulatePhysics);
	}

	// forget about the actor
	GrabbedActor = nullptr;
	Behavior = nullptr;
}

// Called every frame
void UBasicVRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// if an actor is grabbed and a behavior is defined move move him accordingly  
	if (GrabbedActor == nullptr || 	InteractionRayEmitter == nullptr) return;
	
	// if our Grabable Actor is not constrained
	if (Behavior != nullptr)
	{	
		// specifies the hand in space
		const FVector HandPos = InteractionRayEmitter->GetComponentLocation();
		const FQuat HandQuat = InteractionRayEmitter->GetComponentQuat();

		Behavior->HandleNewPositionAndDirection(HandPos, HandQuat); 
	}
}

void UBasicVRInteractionComponent::Initialize(USceneComponent* RayEmitter, float InMaxGrabDistance, float InMaxClickDistance)
{
	if(InteractionRayEmitter!= nullptr) return; /* Return if already initialized */

	InteractionRayEmitter = RayEmitter;
	MaxGrabDistance = InMaxGrabDistance;
	MaxClickDistance = InMaxClickDistance;
}

void UBasicVRInteractionComponent::HandlePhysicsAndAttachActor(AActor* HitActor)
{
	UPrimitiveComponent* PhysicsComp = HitActor->FindComponentByClass<UPrimitiveComponent>();	
	
	bDidSimulatePhysics = PhysicsComp->IsSimulatingPhysics(); // remember if we need to tun physics back on or not	
	PhysicsComp->SetSimulatePhysics(false);
	FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
	HitActor->AttachToComponent(InteractionRayEmitter, Rules);
}

FTwoVectors UBasicVRInteractionComponent::GetHandRay(const float Length) const
{
	const FVector Start = InteractionRayEmitter->GetComponentLocation();
	const FVector Direction = InteractionRayEmitter->GetForwardVector();
	const FVector End = Start + Length * Direction;

	return FTwoVectors(Start, End);
}
