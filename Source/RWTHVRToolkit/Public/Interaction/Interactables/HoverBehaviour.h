// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBehaviour.h"
#include "InteractionEventType.h"
#include "Components/SceneComponent.h"
#include "HoverBehaviour.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHoverEvent, const USceneComponent*, TriggeredComponent,
											   const EInteractionEventType, EventType, const FHitResult&, Hit);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UHoverBehaviour : public UBaseBehaviour
{
	GENERATED_BODY()

public:
	/**
	 * TriggeredComponent: Component that triggered this event (e.g. GrabComponent, RayCastComponent attached at the
	 * VRPawn) Hit: Hit Result of the trace to get access to e.g. contact point/normals etc.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnHoverEvent OnHoverEventEvent;

protected:
	UFUNCTION()
	virtual void OnHoverEvent(const USceneComponent* TriggerComponent, EInteractionEventType EventType,
							  const FHitResult& Hit);

	virtual void BeginPlay() override;
};
