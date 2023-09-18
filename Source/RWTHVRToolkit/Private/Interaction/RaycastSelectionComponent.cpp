// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/RaycastSelectionComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/VirtualRealityUtilities.h"

// Sets default values for this component's properties
URaycastSelectionComponent::URaycastSelectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URaycastSelectionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupInputActions();
	// ...
	
}

// Called every frame
void URaycastSelectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	URaycastSelectable* CurrentSelectable = nullptr;
	
	
	TArray<AActor*> ActorsToIgnore;
	FHitResult Hit;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);
	FVector TraceStart = GetAttachParent()->GetComponentLocation();
	FVector TraceEnd = GetAttachParent()->GetComponentLocation() + TraceLength * GetAttachParent()->GetForwardVector();
	
	auto DebugTrace = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	
	UKismetSystemLibrary::LineTraceSingle(GetWorld(),TraceStart,TraceEnd
		,TraceType,true,ActorsToIgnore,DebugTrace,
		Hit,true,FColor::Green);
	
	AActor* HitActor = Hit.GetActor();
	if(HitActor)
	{
		URaycastSelectable* Selectable = HitActor->FindComponentByClass<URaycastSelectable>();
		if(Selectable && Selectable->IsInteractable)
		{
			CurrentSelectable = Selectable;
			Selectable->HitResult = Hit;
		}
	}


	CurrentRaycastSelectable = CurrentSelectable;
	
	if(CurrentRaycastSelectable != PreviousRaycastSelectable)
	{
		if(CurrentRaycastSelectable)
		{
			CurrentRaycastSelectable->HandleOnHoverStartEvents(this);
		}
		if(PreviousRaycastSelectable)
		{
			PreviousRaycastSelectable->HandleOnHoverEndEvents(this);
		}
	}

	PreviousRaycastSelectable = CurrentRaycastSelectable;
}

void URaycastSelectionComponent::SetupInputActions()
{
	auto InputSubsystem = UVirtualRealityUtilities::GetVRPawnLocalPlayerSubsystem(GetWorld());
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCRaycastSelection,0);
	
	UEnhancedInputComponent* EI = UVirtualRealityUtilities::GetVRPawnInputComponent(GetWorld());
	
	EI->BindAction(RayCastSelectInputAction, ETriggerEvent::Started, this, &URaycastSelectionComponent::OnBeginSelect);
	EI->BindAction(RayCastSelectInputAction, ETriggerEvent::Completed, this, &URaycastSelectionComponent::OnEndSelect);	
}

void URaycastSelectionComponent::OnBeginSelect(const FInputActionValue& Value)
{
	if(CurrentRaycastSelectable)
	{
		CurrentRaycastSelectable->HandleOnClickStartEvents(this, Value);
	}
}

void URaycastSelectionComponent::OnEndSelect(const FInputActionValue& Value)
{
	if(CurrentRaycastSelectable)
		CurrentRaycastSelectable->HandleOnClickEndEvents(this, Value);
}

