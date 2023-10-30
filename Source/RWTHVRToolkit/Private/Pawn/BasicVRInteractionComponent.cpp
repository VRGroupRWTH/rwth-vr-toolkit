// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/BasicVRInteractionComponent.h"

#include "Misc/Optional.h"

DEFINE_LOG_CATEGORY(LogVRInteractionComponent);

// Sets default values for this component's properties
UBasicVRInteractionComponent::UBasicVRInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Setup the interaction ray.
	InteractionRay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Interaction Ray"));
	InteractionRay->SetCastShadow(false);
	// turns off collisions as the InteractionRay is only meant to visualize the ray
	InteractionRay->SetCollisionProfileName(TEXT("NoCollision"));
	// The static mesh reference is set in the blueprint, avoid ConstructorHelpers and hardcoded paths!
	// this ray model has an inlayed cross with flipped normals so it can be seen as a cross in desktop mode where the right hand is attached to the head

	bShowDebug = false; //otherwise the WidgetInteractionComponent debug vis is shown
	InteractionSource = EWidgetInteractionSource::Custom;
	//can also be kept at default (World), this way, however, we efficiently reuse the line traces
}

void UBasicVRInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	//WidgetInteractionComponent
	InteractionDistance = MaxClickDistance;
	InteractionRay->SetRelativeScale3D(FVector(MaxClickDistance / 100.0f, 0.5f, 0.5f));
	//the ray model has a length of 100cm (and is a bit too big in Y/Z dir)
	SetInteractionRayVisibility(InteractionRayVisibility);
}

void UBasicVRInteractionComponent::BeginInteraction()
{
	if (!InteractionRayEmitter) return;

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
}

void UBasicVRInteractionComponent::EndInteraction()
{
	if (!InteractionRayEmitter) return;

	//end interaction of WidgetInteractionComponent
	ReleasePointerKey(EKeys::LeftMouseButton);
}

// Called every frame
void UBasicVRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InteractionRayEmitter) return;

	// only raytrace for targetable objects if bool user wants to enable this feature
	if (!bCanRaytraceEveryTick)
		return;

	const FTwoVectors StartEnd = GetHandRay(MaxClickDistance);
	TOptional<FHitResult> Hit = RaytraceForFirstHit(StartEnd);

	if (!Hit.IsSet() || !Hit->GetActor())
	{
		if (InteractionRayVisibility == EInteractionRayVisibility::VisibleOnHoverOnly)
		{
			InteractionRay->SetVisibility(false);
		}
		return;
	}

	AActor* HitActor = Hit->GetActor();

	// widget interaction
	SetCustomHitResult(Hit.GetValue());
	if (InteractionRayVisibility == EInteractionRayVisibility::VisibleOnHoverOnly)
	{
		if (IsOverInteractableWidget())
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
	if (InteractionRayEmitter) return; /* Return if already initialized */

	InteractionRayEmitter = RayEmitter;

	InteractionRay->AttachToComponent(RayEmitter, FAttachmentTransformRules::KeepRelativeTransform);
	this->AttachToComponent(RayEmitter, FAttachmentTransformRules::KeepRelativeTransform);
}

void UBasicVRInteractionComponent::SetInteractionRayVisibility(EInteractionRayVisibility NewVisibility)
{
	InteractionRayVisibility = NewVisibility;
	if (InteractionRay)
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
		default: ;
		}
	}

	if (InteractionRayVisibility == EInteractionRayVisibility::VisibleOnHoverOnly && !bCanRaytraceEveryTick)
	{
		UE_LOG(LogVRInteractionComponent, Warning,
		       TEXT("VisibleOnHoverOnly needs bCanRaytraceEveryTick=true, so this is set!"));
		bCanRaytraceEveryTick = true;
	}
	if (InteractionRayVisibility == EInteractionRayVisibility::Visible && !bCanRaytraceEveryTick)
	{
		UE_LOG(LogVRInteractionComponent, Warning,
		       TEXT(
			       "VisibleOnHoverOnly will need two clicks to interact with widgets if bCanRaytraceEveryTick is not set!"
		       ));
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
	const FVector End = Ray.v2;

	// will be filled by the Line Trace Function
	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()->GetUniqueID()); // prevents actor hitting itself
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params))
		return {Hit};
	else
		return {};
}
