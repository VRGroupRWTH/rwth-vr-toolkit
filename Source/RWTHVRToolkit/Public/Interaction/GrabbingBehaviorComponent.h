// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GrabbingBehaviorComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginGrabSignature, AActor*, GrabbedBy);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndGrabSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UGrabbingBehaviorComponent : public USceneComponent
{
	GENERATED_BODY()

protected:
	bool WasSimulatingPhysicsOnGrab;
	UPROPERTY()
	UPrimitiveComponent* MyPhysicsComponent;

public:
	UPROPERTY(BlueprintAssignable)
	FOnBeginGrabSignature OnBeginGrab;
	UPROPERTY(BlueprintAssignable)
	FOnEndGrabSignature OnEndGrab;

	// Sets default values for this component's properties
	UGrabbingBehaviorComponent();

public:
	// takes the hand ray and moves the parent actor to a new possible position, also might change rotation
	virtual void HandleGrabHold(FVector Position, FQuat Orientation);

	virtual void HandleGrabStart(AActor* GrabbedBy);

	virtual void HandleGrabEnd();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);
};
