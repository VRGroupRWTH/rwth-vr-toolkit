// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HoverBehaviour.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoverStart, const USceneComponent*, TriggeredComponent, FHitResult, Hit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoverEnd, const USceneComponent*, TriggeredComponent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UHoverBehaviour : public USceneComponent
{
	GENERATED_BODY()

public:	
	UHoverBehaviour();
	/**
	* TriggeredComponent: Component that triggered this event (e.g. GrabComponent, RayCastComponent attached at the VRPawn)
	* Hit: Hit Result of the trace to get access to e.g. contact point/normals etc.
	*/
	UPROPERTY(BlueprintAssignable)
		FOnHoverStart OnHoverStartEvent;

	UPROPERTY(BlueprintAssignable)
		FOnHoverEnd OnHoverEndEvent;

protected:
	UFUNCTION()
		virtual void OnHoverStart(const USceneComponent* TriggeredComponent, FHitResult Hit);
	UFUNCTION()
		virtual void OnHoverEnd(const USceneComponent* TriggeredComponent);
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
