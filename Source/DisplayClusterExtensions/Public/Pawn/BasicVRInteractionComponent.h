// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BasicVRInteractionComponent.generated.h"

class UGrabbingBehaviorComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DISPLAYCLUSTEREXTENSIONS_API UBasicVRInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBasicVRInteractionComponent();

	UFUNCTION(BlueprintCallable) void BeginInteraction(); 
	UFUNCTION(BlueprintCallable) void EndInteraction();   	
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite) float MaxGrabDistance = 50;
	UPROPERTY(BlueprintReadWrite) float MaxClickDistance = 500;
	// Enable this if you want to interact with Targetable classes
	UPROPERTY(EditAnywhere) bool bCanRaytraceEveryTick = false;

	UFUNCTION(BlueprintCallable) void Initialize(USceneComponent* RayEmitter, float InMaxGrabDistance = 50, float InMaxClickDistance = 500);
	
	UFUNCTION(BlueprintCallable, BlueprintPure) AActor* GetGrabbedActor() const { return GrabbedActor;}
	UFUNCTION(BlueprintCallable, BlueprintPure) USceneComponent* GetInteractionRayEmitter() const { return InteractionRayEmitter;	}
private:
	/* Holding a reference to the actor that is currently being grabbed */
	UPROPERTY() AActor* GrabbedActor;
	/* Holds a reference to the grabbed actors physics simulating component if there was one*/
	UPROPERTY() UPrimitiveComponent* ComponentSimulatingPhysics = nullptr;
	UPROPERTY() UGrabbingBehaviorComponent* Behavior = nullptr;
	UPROPERTY() USceneComponent* InteractionRayEmitter = nullptr;
	/* Stores the reference of the Actor that was hit in the last frame*/
	UPROPERTY() AActor* LastActorHit = nullptr;
	void HandlePhysicsAndAttachActor(AActor* HitActor);
	FTwoVectors GetHandRay(float Length) const;
	TOptional<FHitResult> RaytraceForFirstHit(const FTwoVectors& Ray) const;
};

// Free utility functions 
/*
	Returns the UPrimitiveComponent simulating physics that is highest in the hierarchy
*/
UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);
/*
	Recursive Function
	If parent component simulates physics returns GetHighestParentSimulatingPhysics(Parent)
	else returns Comp itself
*/
UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);



