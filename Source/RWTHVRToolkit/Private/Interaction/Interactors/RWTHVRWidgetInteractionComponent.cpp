// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/RWTHVRWidgetInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Interaction/Interactors/DirectInteractionComponent.h"
#include "Logging/StructuredLog.h"
#include "Misc/Optional.h"
#include "Utility/RWTHVRUtilities.h"

URWTHVRWidgetInteractionComponent::URWTHVRWidgetInteractionComponent()
{
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	// Only start ticking once we're initialized
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// Input only, don't tick on DS
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);

	// we have to set this to false, otherwise the component starts ticking on the server
	// it seems like AutoActivation just overrides whatever default tick values I set
	bAutoActivate = false;
}

void URWTHVRWidgetInteractionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());

	// Check if our owner is a Pawn. Currently we call this function from our Pawn, so this is kind of a given.
	// In the future, we might want to support other actors as well.
	if (!Pawn)
	{
		UE_LOGFMT(Toolkit, Warning,
				  "URWTHVRWidgetInteractionComponent::SetupPlayerInput requires a Pawn as Owner, which is not the "
				  "case. Not setting up any input actions.");
		return;
	}

	// Because we cannot use the regular debug ray (only works in editor), we set up our own (mesh) ray.
	SetupInteractionRay();

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (!EI)
	{
		UE_LOGFMT(Toolkit, Warning,
				  "URWTHVRWidgetInteractionComponent::SetupPlayerInput: Cannot cast Pawn's InputComponent to "
				  "UEnhancedInputComponent! Not binding any actions!");
		return;
	}

	if (WidgetLeftClickInputAction)
	{
		EI->BindAction(WidgetLeftClickInputAction, ETriggerEvent::Started, this,
					   &URWTHVRWidgetInteractionComponent::OnBeginLeftClick);
		EI->BindAction(WidgetLeftClickInputAction, ETriggerEvent::Completed, this,
					   &URWTHVRWidgetInteractionComponent::OnEndLeftClick);
	}
	if (WidgetRightClickInputAction)
	{
		EI->BindAction(WidgetRightClickInputAction, ETriggerEvent::Started, this,
					   &URWTHVRWidgetInteractionComponent::OnBeginRightClick);
		EI->BindAction(WidgetRightClickInputAction, ETriggerEvent::Completed, this,
					   &URWTHVRWidgetInteractionComponent::OnEndRightClick);
	}
}

// Called every frame
void URWTHVRWidgetInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
													  FActorComponentTickFunction* ThisTickFunction)
{
	// We should only tick on the local owner (the controlling client). Not on the server, not on any other pawn.
	// In theory, this should never happen as we only activate the component for the local player anyway.
	// But sometimes Unreal is strange and this still slips through, catch it if it happens.
	if (!GetOwner() || !GetOwner()->HasLocalNetOwner())
	{
		UE_LOGFMT(Toolkit, Error, "VRWidgetInteraction Ticking on non-owner! Deactivating!");
		SetComponentTickEnabled(false);
		Deactivate();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Disable/enable ray on hover - SetVisibility is smart enough to check for change
	if (InteractionRayVisibility == EInteractionRayVisibility::VisibleOnHoverOnly && InteractionRay)
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

void URWTHVRWidgetInteractionComponent::SetInteractionRayVisibility(EInteractionRayVisibility NewVisibility)
{
	InteractionRayVisibility = NewVisibility;

	if (InteractionRay)
		InteractionRay->SetVisibility(NewVisibility == Visible);
	else
		UE_LOGFMT(Toolkit, Error,
				  "URWTHVRWidgetInteractionComponent::SetInteractionRayVisibility: InteractionRay not set yet!");
}

// Forward the click to the WidgetInteraction
void URWTHVRWidgetInteractionComponent::OnBeginLeftClick(const FInputActionValue& Value)
{
	PressPointerKey(EKeys::LeftMouseButton);
}

// Forward the end click to the WidgetInteraction
void URWTHVRWidgetInteractionComponent::OnEndLeftClick(const FInputActionValue& Value)
{
	ReleasePointerKey(EKeys::LeftMouseButton);
}

// Forward the click to the WidgetInteraction
void URWTHVRWidgetInteractionComponent::OnBeginRightClick(const FInputActionValue& Value)
{
	PressPointerKey(EKeys::RightMouseButton);
}

// Forward the end click to the WidgetInteraction
void URWTHVRWidgetInteractionComponent::OnEndRightClick(const FInputActionValue& Value)
{
	ReleasePointerKey(EKeys::RightMouseButton);
}

void URWTHVRWidgetInteractionComponent::CreateInteractionRay()
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
}

void URWTHVRWidgetInteractionComponent::SetupInteractionRay()
{
	// Create the InteractionRay component and attach it to us.
	CreateInteractionRay();

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

	// the ray model has a length of 100cm (and is a bit too big in Y/Z dir)
	InteractionRay->SetRelativeScale3D(FVector(InteractionDistance / 100.0f, 0.5f, 0.5f));
	SetInteractionRayVisibility(InteractionRayVisibility);

	// We are set up, enable ticking
	// As we disabled auto activation, we need to activate the component manually here.
	Activate();
	SetComponentTickEnabled(true);
}
