// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactees/ActionBehaviour.h"

// We disable ticking here, as we are mainly interested in the events
UActionBehaviour::UActionBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UActionBehaviour::OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
                                     const FInputActionValue& Value)
{
}

void UActionBehaviour::OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
                                   const FInputActionValue& Value)
{
}

void UActionBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnActionBeginEvent.AddDynamic(this, &UActionBehaviour::OnActionStart);
	OnActionEndEvent.AddDynamic(this, &UActionBehaviour::OnActionEnd);
}

// Called every frame
void UActionBehaviour::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
