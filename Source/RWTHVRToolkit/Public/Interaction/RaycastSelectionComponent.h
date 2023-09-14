// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RaycastSelectable.h"
#include "Components/SceneComponent.h"
#include "RaycastSelectionComponent.generated.h"


UCLASS(Abstract,Blueprintable)
class RWTHVRTOOLKIT_API URaycastSelectionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URaycastSelectionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMCRaycastSelection;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* RayCastSelectInputAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Raycast")
	float TraceLength = 3000.0;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Raycast")
	bool bShowDebugTrace = false;

private:
	void SetupInputActions();
	
	UFUNCTION()
	void OnBeginSelect(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndSelect(const FInputActionValue& Value);

	UPROPERTY()
	URaycastSelectable* PreviousRaycastSelectable;

	UPROPERTY()
	URaycastSelectable* CurrentRaycastSelectable;
		
};
