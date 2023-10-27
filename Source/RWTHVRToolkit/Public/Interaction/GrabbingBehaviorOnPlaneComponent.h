// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbingBehaviorComponent.h"
#include "GrabbingBehaviorOnPlaneComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UGrabbingBehaviorOnPlaneComponent : public UGrabbingBehaviorComponent
{
	GENERATED_BODY()

public:	

	// defining the constraint plane with these 3 parameters
	UFUNCTION(BlueprintCallable) void SetDistance(float Dist);
	UFUNCTION(BlueprintCallable) float GetDistance() const;
	
	virtual void HandleGrabHold(FVector position, FQuat orientation) override;
	
private:
	UPROPERTY(EditAnywhere) float Distance; // distance the object can be moved from the center 

};
