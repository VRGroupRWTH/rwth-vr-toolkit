// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbingBehaviorComponent.h"
#include "GrabbingBehaviorOnLineComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DISPLAYCLUSTEREXTENSIONS_API UGrabbingBehaviorOnLineComponent : public UGrabbingBehaviorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabbingBehaviorOnLineComponent();

	// defining a constraint line with these 3 parameters
	UFUNCTION(BlueprintCallable) void SetDistance(float Dist);
	UFUNCTION(BlueprintCallable) float GetDistance() const;

	virtual void HandleNewPositionAndDirection(FVector position, FQuat orientation) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	UPROPERTY(EditAnywhere)	float Distance; // distance the object can be moved from the center 

};
