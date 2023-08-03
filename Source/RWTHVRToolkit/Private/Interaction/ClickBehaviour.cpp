// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/ClickBehaviour.h"

// Sets default values for this component's properties
UClickBehaviour::UClickBehaviour()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClickBehaviour::OnClickStart(const UIntenSelectComponent* IntenSelect, const FVector& Point)
{
	
}

void UClickBehaviour::OnClickEnd(const UIntenSelectComponent* IntenSelect)
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

