// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/HoverBehaviour.h"

// Sets default values for this component's properties
UHoverBehaviour::UHoverBehaviour()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UHoverBehaviour::OnHoverStart(const USceneComponent* TriggeredComponent, FHitResult Hit)
{
}

void UHoverBehaviour::OnHoverEnd(const USceneComponent* TriggeredComponent)
{
}


void UHoverBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnHoverStartEvent.AddDynamic(this, &UHoverBehaviour::OnHoverStart);
	OnHoverEndEvent.AddDynamic(this, &UHoverBehaviour::OnHoverEnd);
	
}


void UHoverBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

