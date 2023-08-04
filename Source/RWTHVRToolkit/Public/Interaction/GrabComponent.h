// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GrabComponent.generated.h"

class UGrabbableComponent;

UCLASS(Abstract,Blueprintable)
class RWTHVRTOOLKIT_API UGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMCGrab;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* GrabInputAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grabbing")
	float GrabSphereRadius = 15.0;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Grabbing")
	bool bShowDebugTrace = true;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void SetupInputActions();
	
	UFUNCTION()
	void OnBeginGrab(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndGrab(const FInputActionValue& Value);

	UPROPERTY()
	TArray<UGrabbableComponent*> PreviousGrabbablesInRange;

	UPROPERTY()
	TArray<UGrabbableComponent*> CurrentGrabbableInRange;

		
};
