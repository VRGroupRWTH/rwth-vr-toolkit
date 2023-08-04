// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "InputActionValue.h"
#include "ClickBehaviour.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClickStart, USceneComponent*, TriggeredComponent, const FInputActionValue&, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClicktEnd, USceneComponent*, TriggeredComponent, const FInputActionValue&, Value);

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
		virtual void OnClickStart(USceneComponent* TriggeredComponent,const FInputActionValue& Value);
	UFUNCTION()
		virtual void OnClickEnd(USceneComponent* TriggeredComponent,const FInputActionValue& Value);
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
