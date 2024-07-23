// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "UBaseInteractionComponent.generated.h"


struct FInputActionValue;
enum EInteractionEventType : uint8;
class UActionBehaviour;
class UHoverBehaviour;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UUBaseInteractionComponent : public USceneComponent, public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUBaseInteractionComponent();

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* InteractionInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Interaction")
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Interaction")
	bool bShowDebugTrace = false;

	/*
	 * Replication part
	 */


	/**
	 * Requests the start of the replication process for the given HoverBehaviour, EventType and Hit.
	 * Depending on authority, this executes the replication of the behaviour directly or requests it via a server RPC.
	 */
	void RequestHoverBehaviourReplicationStart(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
											   const FHitResult& Hit);

	/**
	 * This is executed on the server/authority. The behaviour is actually executed directly on the server, or
	 * multicast to all connected clients. The multicast then executes the behaviour.
	 */
	void HoverBehaviourReplicationStart(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
										const FHitResult& Hit);

	/**
	 * This is only executed on the local client which processed the interaction and requested the replication process
	 * to be started. Can be used e.g. for local effects or things that should be done both on the server and local client.
	 */
	void HoverBehaviourReplicationOriginatorCallback(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
										const FHitResult& Hit);

	/**
	 * Requests the start of the replication process for the given ActionBehaviour, EventType and the Value of the Input Action.
	 * Depending on authority, this executes the replication of the behaviour directly or requests it via a server RPC. 
	 */
	void RequestActionBehaviourReplicationStart(UActionBehaviour* Behaviour, const EInteractionEventType EventType,
												const FInputActionValue& Value);

	/**
	 * This is executed on the server/authority. The behaviour is actually executed directly on the server, or
	 * multicast to all connected clients. The multicast then executes the behaviour.
	 */
	void ActionBehaviourReplicationStart(UActionBehaviour* Behaviour, const EInteractionEventType EventType,
										 const FInputActionValue& Value);
	

	// RPCs
	UFUNCTION(Server, Reliable)
	void ServerRequestHoverBehaviourReplicationStartRpc(UHoverBehaviour* Behaviour,
														const EInteractionEventType EventType, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHoverBehaviourReplicationStartRpc(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
													const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void ServerRequestActionBehaviourReplicationStartRpc(UActionBehaviour* Behaviour,
														 const EInteractionEventType EventType,
														 const FInputActionValue& Value);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActionBehaviourReplicationStartRpc(UActionBehaviour* Behaviour, const EInteractionEventType EventType,
													 const FInputActionValue& Value);

private:
	UFUNCTION()
	virtual void OnBeginInteractionInputAction(const FInputActionValue& Value);

	UFUNCTION()
	virtual void OnEndInteractionInputAction(const FInputActionValue& Value);
};
