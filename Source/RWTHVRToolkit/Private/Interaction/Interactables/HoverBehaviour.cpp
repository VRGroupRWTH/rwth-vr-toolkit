// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/HoverBehaviour.h"

void UHoverBehaviour::OnHoverStart(const USceneComponent* TriggeredComponent, FHitResult Hit) {}

void UHoverBehaviour::OnHoverEnd(const USceneComponent* TriggeredComponent) {}

void UHoverBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnHoverStartEvent.AddDynamic(this, &UHoverBehaviour::OnHoverStart);
	OnHoverEndEvent.AddDynamic(this, &UHoverBehaviour::OnHoverEnd);
}
