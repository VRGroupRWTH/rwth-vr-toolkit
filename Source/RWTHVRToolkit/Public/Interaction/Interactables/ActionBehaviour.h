// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBehaviour.h"
#include "Components/SceneComponent.h"
#include "InputActionValue.h"
#include "ActionBehaviour.generated.h"

enum EInteractionEventType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActionBegin, USceneComponent*, TriggerComponent,
											   const EInteractionEventType, EventType, const FInputActionValue&, Value);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UActionBehaviour : public UBaseBehaviour
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActionBehaviour();

	UPROPERTY(BlueprintAssignable)
	FOnActionBegin OnActionEventEvent;

	/**
	 * Replication specific:
	 * This is only executed on the local client which processed the interaction and requested the replication process
	 * to be started. Can be used e.g. for local effects or things that should be done both on the server and local
	 * client. Broadcast by UInteractableComponent when the originating client sends a server rpc to start the
	 * interaction replication.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnActionBegin OnActionReplicationStartedOriginatorEvent;


protected:
	UFUNCTION()
	virtual void OnActionEvent(USceneComponent* TriggerComponent, const EInteractionEventType EventType,
							   const FInputActionValue& Value);

	virtual void BeginPlay() override;
};
