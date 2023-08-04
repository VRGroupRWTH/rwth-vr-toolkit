// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClickBehaviour.h"
#include "Components/SceneComponent.h"
#include "OnClickGrabBehavior.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UOnClickGrabBehavior : public UClickBehaviour
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOnClickGrabBehavior();

	UPROPERTY(EditAnywhere, Category="Grabbing")
	bool bBlockOtherInteractionsWhileGrabbed = true;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void OnClickStart(USceneComponent* TriggeredComponent, const FInputActionValue& Value) override;
	virtual void OnClickEnd(USceneComponent* TriggeredComponent, const FInputActionValue& Value) override;
	
	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UPROPERTY()
	UPrimitiveComponent* MyPhysicsComponent;


		
};
