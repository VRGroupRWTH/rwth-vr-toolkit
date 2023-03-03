// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/BasicVRInteractionComponent.h"


#include "Interaction/Clickable.h"
#include "Interaction/Grabable.h"
#include "Interaction/Targetable.h"
#include "Interaction/GrabbingBehaviorComponent.h"
#include "Misc/Optional.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"

DEFINE_LOG_CATEGORY(LogVRInteractionComponent);

// Sets default values for this component's properties
UBasicVRInteractionComponent::UBasicVRInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Setup the interaction ray.
	InteractionRay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Interaction Ray"));
	//this ray model has an inlayed cross with flipped normals so it can be seen as a cross in desktop mode where the right hand is attached to the head
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/RWTHVRToolkit/PointingRay/Ray_Mesh"));
	if (MeshAsset.Object != nullptr)
	{
		InteractionRay->SetStaticMesh(MeshAsset.Object);
	}
	// turns off collisions as the InteractionRay is only meant to visualize the ray
	InteractionRay->SetCollisionProfileName(TEXT("NoCollision"));
	bShowDebug = false; //otherwise the WidgetInteractionComponent debug vis is shown
	InteractionSource = EWidgetInteractionSource::Custom; //can also be kept at default (World), this way, however, we efficiently reuse the line traces
	
}

void UBasicVRInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	//WidgetInteractionComponent
	InteractionDistance = MaxClickDistance;
	InteractionRay->SetRelativeScale3D(FVector(MaxClickDistance / 100.0f, 0.5f, 0.5f)); //the ray model has a length of 100cm (and is a bit too big in Y/Z dir)
	SetInteractionRayVisibility(InteractionRayVisibility);
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

	//trigger interaction of WidgetInteractionComponent
	SetCustomHitResult(Hit.GetValue());
	//if !bCanRaytraceEveryTick, you have to click twice, since the first tick it only highlights and can't directly click
	PressPointerKey(EKeys::LeftMouseButton);
	
	
	if (HitActor && HitActor->Implements<UGrabable>() && Hit->Distance < MaxGrabDistance)
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
	else if (HitActor && HitActor->Implements<UClickable>() && Hit->Distance < MaxClickDistance)
	{
		IClickable::Execute_OnClick(HitActor, Hit->Location);
	}
}

void UBasicVRInteractionComponent::EndInteraction()
{
	if(!InteractionRayEmitter) return;

	//end interaction of WidgetInteractionComponent
	ReleasePointerKey(EKeys::LeftMouseButton);
	
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

	if (!Hit.IsSet() || !Hit->GetActor())
	{
		if(InteractionRayVisibility==EInteractionRayVisibility::VisibleOnHoverOnly)
		{
			InteractionRay->SetVisibility(false);
		}

		// Execute leave event on the actor that lost the focus if there was one
		if (LastActorHit && LastActorHit->Implements<UTargetable>())
		{
				ITargetable::Execute_OnTargetedLeave(LastActorHit);
		}

		LastActorHit = nullptr;
		return;
	}
	
	AActor* HitActor = Hit->GetActor();

	// Execute Leave and enter events when the focused actor changed
	if (HitActor != LastActorHit)
	{
		//We can always execute the enter event as we are sure that a hit occured
		if (HitActor->Implements<UTargetable>())
		{
			ITargetable::Execute_OnTargetedEnter(HitActor);
		}

		//Only execute the Leave Event if there was an actor that was focused previously
		if (LastActorHit != nullptr && LastActorHit->Implements<UTargetable>())
		{
			ITargetable::Execute_OnTargetedLeave(LastActorHit);
		}
	}

	// for now uses the same distance as clicking
	if (HitActor->Implements<UTargetable>() && Hit->Distance < MaxClickDistance)
	{
		ITargetable::Execute_OnTargeted(HitActor, Hit->Location);
	}

	// widget interaction
	SetCustomHitResult(Hit.GetValue());
	if(InteractionRayVisibility==EInteractionRayVisibility::VisibleOnHoverOnly)
	{
		if(HitActor->Implements<UTargetable>() || HitActor->Implements<UClickable>() || IsOverInteractableWidget())
		{
			InteractionRay->SetVisibility(true);
		}
		else
		{
			InteractionRay->SetVisibility(false);
		}
	}
	LastActorHit = HitActor; // Store the actor that was hit to have access to it in the next frame as well
}

void UBasicVRInteractionComponent::Initialize(USceneComponent* RayEmitter)
{
	if(InteractionRayEmitter) return; /* Return if already initialized */

	InteractionRayEmitter = RayEmitter;

	InteractionRay->AttachToComponent(RayEmitter, FAttachmentTransformRules::KeepRelativeTransform);
	this->AttachToComponent(RayEmitter, FAttachmentTransformRules::KeepRelativeTransform);
}

void UBasicVRInteractionComponent::SetInteractionRayVisibility(EInteractionRayVisibility NewVisibility)
{
	InteractionRayVisibility = NewVisibility;
	if(InteractionRay)
	{
		switch (InteractionRayVisibility)
		{
		case Visible:
			InteractionRay->SetVisibility(true);
			break;
		case VisibleOnHoverOnly:
		case Invisible:
			InteractionRay->SetVisibility(false);
			break;
		}
	}

	if(InteractionRayVisibility==EInteractionRayVisibility::VisibleOnHoverOnly && !bCanRaytraceEveryTick)
	{
		UE_LOG(LogVRInteractionComponent, Warning, TEXT("VisibleOnHoverOnly needs bCanRaytraceEveryTick=true, so this is set!"));
		bCanRaytraceEveryTick=true;
	}
	if(InteractionRayVisibility==EInteractionRayVisibility::Visible && !bCanRaytraceEveryTick)
	{
		UE_LOG(LogVRInteractionComponent, Warning, TEXT("VisibleOnHoverOnly will need two clicks to interact with widgets if bCanRaytraceEveryTick is not set!"));
	}
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

	FCollisionQueryParams Params; 
	Params.AddIgnoredActor(GetOwner()->GetUniqueID()); // prevents actor hitting itself
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility,Params))
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
