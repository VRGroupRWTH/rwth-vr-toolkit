// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/ActionBehaviour.h"

// We disable ticking here, as we are mainly interested in the events
UActionBehaviour::UActionBehaviour() { PrimaryComponentTick.bCanEverTick = false; }

void UActionBehaviour::OnActionEvent(USceneComponent* TriggerComponent, const EInteractionEventType EventType,
									 const FInputActionValue& Value)
{
}

void UActionBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnActionEventEvent.AddDynamic(this, &UActionBehaviour::OnActionEvent);
}
