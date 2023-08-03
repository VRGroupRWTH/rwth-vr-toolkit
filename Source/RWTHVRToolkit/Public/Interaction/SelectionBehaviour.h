// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SelectionBehaviour.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectStart, const UIntenSelectComponent*, IntenSelect, const FVector&, GrabbedPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectEnd, const UIntenSelectComponent*, IntenSelect);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API USelectionBehaviour : public USceneComponent
{
	GENERATED_BODY()

public:	
	USelectionBehaviour();
	
	UPROPERTY(BlueprintAssignable)
		FOnSelectStart OnSelectStartEvent;
	UPROPERTY(BlueprintAssignable)
		FOnSelectEnd OnSelectEndEvent;

protected:
	UFUNCTION()
		virtual void OnSelectStart(const UIntenSelectComponent* IntenSelect, const FVector& Point);
	UFUNCTION()
		virtual void OnSelectEnd(const UIntenSelectComponent* IntenSelect);
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
