// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GrabbingBehaviorComponent.generated.h"


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UGrabbingBehaviorComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabbingBehaviorComponent();


	// takes the hand ray and moves the parent actor to a new possible position, also might change rotation
	virtual void HandleNewPositionAndDirection(FVector position, FQuat orientation) PURE_VIRTUAL(UGrabbingBehaviorComponent::GeneratePossiblePosition,);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
