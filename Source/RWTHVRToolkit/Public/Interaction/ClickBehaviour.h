// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Pawn/IntenSelectComponent.h"
#include "ClickBehaviour.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClickStart, const UIntenSelectComponent*, IntenSelect, const FVector&, Point);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClicktEnd, const UIntenSelectComponent*, IntenSelect);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UClickBehaviour : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClickBehaviour();

	UPROPERTY(BlueprintAssignable)
		FOnClickStart OnClickStartEvent;
	UPROPERTY(BlueprintAssignable)
		FOnClicktEnd OnClickEndEvent;

protected:
	UFUNCTION()
		virtual void OnClickStart(const UIntenSelectComponent* IntenSelect, const FVector& Point);
	UFUNCTION()
		virtual void OnClickEnd(const UIntenSelectComponent* IntenSelect);
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
