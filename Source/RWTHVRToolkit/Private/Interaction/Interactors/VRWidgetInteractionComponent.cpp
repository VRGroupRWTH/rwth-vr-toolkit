// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/VRWidgetInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/Interactors/GrabComponent.h"
#include "Misc/Optional.h"
#include "Utility/VirtualRealityUtilities.h"

UVRWidgetInteractionComponent::UVRWidgetInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRWidgetInteractionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;

	auto* InputSubsystem = GetEnhancedInputLocalPlayerSubsystem(Pawn);
	if (!InputSubsystem)
		return;

	// Because we cannot use the regular debug ray (only works in editor), we set up our own (mesh) ray.
	SetupInteractionRay();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCWidgetInteraction, 0);

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (EI == nullptr)
		return;

	EI->BindAction(WidgetClickInputAction, ETriggerEvent::Started, this, &UVRWidgetInteractionComponent::OnBeginClick);
	EI->BindAction(WidgetClickInputAction, ETriggerEvent::Completed, this, &UVRWidgetInteractionComponent::OnEndClick);
}

// Called every frame
void UVRWidgetInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// only raytrace for targetable objects if user wants to enable this feature
	if (!bCanRaytraceEveryTick)
		return;

	// Disable/enable ray on hover
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
}

void UVRWidgetInteractionComponent::SetInteractionRayVisibility(EInteractionRayVisibility NewVisibility)
{
	InteractionRayVisibility = NewVisibility;
	InteractionRay->SetVisibility(NewVisibility == Visible);

	if (InteractionRayVisibility == EInteractionRayVisibility::VisibleOnHoverOnly && !bCanRaytraceEveryTick)
	{
		UE_LOG(Toolkit, Warning,
		       TEXT("VisibleOnHoverOnly needs bCanRaytraceEveryTick=true, so this is forcibly set!"));
		bCanRaytraceEveryTick = true;
	}
	if (InteractionRayVisibility == EInteractionRayVisibility::Visible && !bCanRaytraceEveryTick)
	{
		UE_LOG(Toolkit, Warning,
		       TEXT(
			       "VisibleOnHoverOnly will need two clicks to interact with widgets if bCanRaytraceEveryTick is not set!"
		       ));
	}
}

// Forward the click to the WidgetInteraction
void UVRWidgetInteractionComponent::OnBeginClick(const FInputActionValue& Value)
{
	PressPointerKey(EKeys::LeftMouseButton);
}

// Forward the end click to the WidgetInteraction
void UVRWidgetInteractionComponent::OnEndClick(const FInputActionValue& Value)
{
	ReleasePointerKey(EKeys::LeftMouseButton);
}

void UVRWidgetInteractionComponent::SetupInteractionRay()
{
	// Only create a new static mesh component if we haven't gotten one already
	if (!InteractionRay)
	{
		// Create the new component
		InteractionRay = NewObject<UStaticMeshComponent>(this);

		// If we're not already attached, attach. Not sure why this is needed, taken from engine code.
		if (!InteractionRay->GetAttachParent() && !InteractionRay->IsAttachedTo(this))
		{
			const AActor* Owner = GetOwner();

			// Copied from engine code - I don't think this case will ever happen
			if (!Owner || !Owner->GetWorld())
			{
				if (UWorld* World = GetWorld())
				{
					InteractionRay->RegisterComponentWithWorld(World);
					InteractionRay->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
				else
				{
					InteractionRay->SetupAttachment(this);
				}
			}
			else
			{
				// Usual case, just attach it to ourselves.
				InteractionRay->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				InteractionRay->RegisterComponent();
			}
		}
	}

	if (InteractionRay)
	{
		// Setup the interaction ray.
		// The static mesh reference is set in the blueprint, avoid ConstructorHelpers and hardcoded paths!
		// this ray model has an inlayed cross with flipped normals so it can be seen as a cross in desktop mode
		// where the right hand is attached to the head

		if (InteractionRayMesh)
		{
			InteractionRay->SetStaticMesh(InteractionRayMesh);
		}

		InteractionRay->SetCastShadow(false);
		// turns off collisions as the InteractionRay is only meant to visualize the ray
		InteractionRay->SetCollisionProfileName(TEXT("NoCollision"));

		//the ray model has a length of 100cm (and is a bit too big in Y/Z dir)
		InteractionRay->SetRelativeScale3D(FVector(InteractionDistance / 100.0f, 0.5f, 0.5f));
		SetInteractionRayVisibility(InteractionRayVisibility);
	}
}
