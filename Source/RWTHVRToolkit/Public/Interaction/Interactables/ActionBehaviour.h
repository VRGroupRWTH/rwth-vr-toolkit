// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/SceneComponent.h"
#include "InputActionValue.h"
#include "InteractionBitSet.h"
#include "ActionBehaviour.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActionBegin, USceneComponent*, TriggeredComponent,
                                               const UInputAction*,
                                               InputAction, const FInputActionValue&, Value);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActionEnd, USceneComponent*, TriggeredComponent, const UInputAction*,
                                               InputAction, const FInputActionValue&, Value);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UActionBehaviour : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActionBehaviour();

	UPROPERTY(BlueprintAssignable)
	FOnActionBegin OnActionBeginEvent;

	UPROPERTY(BlueprintAssignable)
	FOnActionEnd OnActionEndEvent;

protected:
	UFUNCTION()
	virtual void OnActionStart(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
	                           const FInputActionValue& Value);

	UFUNCTION()
	virtual void OnActionEnd(USceneComponent* TriggeredComponent, const UInputAction* InputAction,
	                         const FInputActionValue& Value);

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
