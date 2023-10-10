// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/ClickBehaviour.h"

// Sets default values for this component's properties
UClickBehaviour::UClickBehaviour()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClickBehaviour::OnClickStart(USceneComponent* TriggeredComponent,const FInputActionValue& Value)
{
	Value.
}

void UClickBehaviour::OnClickEnd(USceneComponent* TriggeredComponent,const FInputActionValue& Value)
{
	
}

void UClickBehaviour::BeginPlay()
{
	Super::BeginPlay();

	OnClickStartEvent.AddDynamic(this, &UClickBehaviour::OnClickStart);
	OnClickEndEvent.AddDynamic(this, &UClickBehaviour::OnClickEnd);
}


// Called every frame
void UClickBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

