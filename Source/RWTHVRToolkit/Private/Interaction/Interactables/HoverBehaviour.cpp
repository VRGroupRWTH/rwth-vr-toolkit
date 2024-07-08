// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/HoverBehaviour.h"


void UHoverBehaviour::OnHoverEvent(const USceneComponent* TriggerComponent, EInteractionEventType EventType,
								   const FHitResult& Hit)
{
}
void UHoverBehaviour::BeginPlay()
{
	Super::BeginPlay();
	OnHoverEventEvent.AddDynamic(this, &UHoverBehaviour::OnHoverEvent);
}
