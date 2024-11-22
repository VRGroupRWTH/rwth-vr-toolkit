// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/ReelableNoAttachGrabBehavior.h"


#include "EnhancedInputComponent.h"
#include "InputActionValue.h"


// Sets default values for this component's properties
UReelableNoAttachGrabBehavior::UReelableNoAttachGrabBehavior()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UReelableNoAttachGrabBehavior::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UReelableNoAttachGrabBehavior::TickComponent(float DeltaTime, ELevelTick TickType,
												  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UReelableNoAttachGrabBehavior::OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
												  const FInputActionValue& Value)
{
	Super::OnActionStart(TriggeredComponent, InputAction, Value);
	// Check if the OwnerActor can handle input
	if (!ReelInputAction)
	{
		return;
	}
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		GetOwner()->EnableInput(PlayerController);

		// Bind the input action if the actor's input component is EnhancedInputComponent
		if (!ActionBound)
		{
			if (UEnhancedInputComponent* EnhancedInputComponent =
					Cast<UEnhancedInputComponent>(GetOwner()->InputComponent))
			{
				EnhancedInputComponent->BindAction(ReelInputAction, ETriggerEvent::Triggered, this,
												   &UReelableNoAttachGrabBehavior::ReelAction);
				ActionBound = true;
			}
		}
	}
	BoundingSphereRadius = ParentActor->GetComponentsBoundingBox(true, true).GetExtent().Length();
}
void UReelableNoAttachGrabBehavior::OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
												const FInputActionValue& Value)
{
	Super::OnActionEnd(TriggeredComponent, InputAction, Value);

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		GetOwner()->DisableInput(PlayerController);
	}
}

void UReelableNoAttachGrabBehavior::ReelAction(const FInputActionValue& Value)
{
	FVector VectorToHand = (ProxyComponent->GetComponentLocation() - CurrentAttachParent->GetComponentLocation());
	double VectorToHandLength = VectorToHand.Length();
	VectorToHand.Normalize();

	auto Offset = VectorToHand * Value.Get<float>() * ReelSpeed;
	if ((Value.Get<float>() > 0) || (VectorToHandLength > (BoundingSphereRadius + HandDeadZone)))
	{
		ProxyComponent->AddWorldOffset(Offset);
	}
}
