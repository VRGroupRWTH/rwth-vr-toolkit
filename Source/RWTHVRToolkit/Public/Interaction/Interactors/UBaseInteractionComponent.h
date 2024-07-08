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

	void RequestHoverBehaviourReplicationStart(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
											   const FHitResult& Hit);
	void HoverBehaviourReplicationStart(UHoverBehaviour* Behaviour, const EInteractionEventType EventType,
										const FHitResult& Hit);

	void RequestActionBehaviourReplicationStart(UActionBehaviour* Behaviour, const EInteractionEventType EventType,
												const FInputActionValue& Value);
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
