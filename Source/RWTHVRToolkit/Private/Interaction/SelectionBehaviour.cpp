// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/SelectionBehaviour.h"

// Sets default values for this component's properties
USelectionBehaviour::USelectionBehaviour()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void USelectionBehaviour::OnSelectStart(const UIntenSelectComponent* IntenSelect, const FVector& Point)
{
}

void USelectionBehaviour::OnSelectEnd(const UIntenSelectComponent* IntenSelect)
{
}


void USelectionBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnSelectStartEvent.AddDynamic(this, &USelectionBehaviour::OnSelectStart);
	OnSelectEndEvent.AddDynamic(this, &USelectionBehaviour::OnSelectEnd);
	
}


void USelectionBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

