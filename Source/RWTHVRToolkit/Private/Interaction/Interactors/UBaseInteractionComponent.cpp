// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactors/UBaseInteractionComponent.h"

#include "Interaction/Interactables/ActionBehaviour.h"
#include "Interaction/Interactables/HoverBehaviour.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

// Sets default values for this component's properties
UUBaseInteractionComponent::UUBaseInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
	// features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}

void UUBaseInteractionComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	IInputExtensionInterface::SetupPlayerInput(PlayerInputComponent);
}

void UUBaseInteractionComponent::OnBeginInteractionInputAction(const FInputActionValue& Value) {}

void UUBaseInteractionComponent::OnEndInteractionInputAction(const FInputActionValue& Value) {}

void UUBaseInteractionComponent::RequestHoverBehaviourReplicationStart(UHoverBehaviour* Behaviour,
																	   const EInteractionEventType EventType,
																	   const FHitResult& Hit)
{
	if (GetOwner()->HasLocalNetOwner())
	{
		if (GetOwner()->HasAuthority())
		{
			HoverBehaviourReplicationStart(Behaviour, EventType, Hit);
		}
		else
		{
			// RPC
			ServerRequestHoverBehaviourReplicationStartRpc(Behaviour, EventType, Hit);
		}
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "Interaction: Trying to replicate HoverBehaviour {HoverBehaviour} in InteractionComponent "
				  "{InteractionComponent} which has no local net owner!",
				  Behaviour->GetName(), this->GetName());
	}
}

void UUBaseInteractionComponent::HoverBehaviourReplicationStart(UHoverBehaviour* Behaviour,
																const EInteractionEventType EventType,
																const FHitResult& Hit)
{
	if (!Behaviour->bExecuteOnServer) // this should never happen
	{
		UE_LOGFMT(Toolkit, Error,
				  "Interaction: Trying to execute HoverBehaviour {HoverBehaviour} in InteractionComponent "
				  "{InteractionComponent} on the server, but bExecuteOnServer is false!",
				  Behaviour->GetName(), this->GetName());
		return;
	}

	// If desired, multicast to all clients (including originator)
	if (Behaviour->bExecuteOnAllClients)
	{
		MulticastHoverBehaviourReplicationStartRpc(Behaviour, EventType, Hit);
	}
	else
	{
		// Execute the behaviour only on the server
		Behaviour->OnHoverEventEvent.Broadcast(this, EventType, Hit);
	}
}

void UUBaseInteractionComponent::ServerRequestHoverBehaviourReplicationStartRpc_Implementation(
	UHoverBehaviour* Behaviour, const EInteractionEventType EventType, const FHitResult& Hit)
{
	HoverBehaviourReplicationStart(Behaviour, EventType, Hit);
}

void UUBaseInteractionComponent::MulticastHoverBehaviourReplicationStartRpc_Implementation(
	UHoverBehaviour* Behaviour, const EInteractionEventType EventType, const FHitResult& Hit)
{
	Behaviour->OnHoverEventEvent.Broadcast(this, EventType, Hit);
}

void UUBaseInteractionComponent::RequestActionBehaviourReplicationStart(UActionBehaviour* Behaviour,
																		const EInteractionEventType EventType,
																		const FInputActionValue& Value)
{
	if (GetOwner()->HasLocalNetOwner())
	{
		if (GetOwner()->HasAuthority())
		{
			ActionBehaviourReplicationStart(Behaviour, EventType, Value);
		}
		else
		{
			// RPC
			ServerRequestActionBehaviourReplicationStartRpc(Behaviour, EventType, Value);
		}
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "Interaction: Trying to replicate HoverBehaviour {HoverBehaviour} in InteractionComponent "
				  "{InteractionComponent} which has no local net owner!",
				  Behaviour->GetName(), this->GetName());
	}
}

void UUBaseInteractionComponent::ActionBehaviourReplicationStart(UActionBehaviour* Behaviour,
																 const EInteractionEventType EventType,
																 const FInputActionValue& Value)
{
	if (!Behaviour->bExecuteOnServer) // this should never happen
	{
		UE_LOGFMT(Toolkit, Error,
				  "Interaction: Trying to execute ActionBehaviour {ActionBehaviour} in InteractionComponent "
				  "{InteractionComponent} on the server, but bExecuteOnServer is false!",
				  Behaviour->GetName(), this->GetName());
		return;
	}

	// If desired, multicast to all clients (including originator)
	if (Behaviour->bExecuteOnAllClients)
	{
		MulticastActionBehaviourReplicationStartRpc(Behaviour, EventType, Value);
	}
	else
	{
		// Execute the behaviour only on the server
		Behaviour->OnActionEventEvent.Broadcast(this, EventType, Value);
	}
}

void UUBaseInteractionComponent::ServerRequestActionBehaviourReplicationStartRpc_Implementation(
	UActionBehaviour* Behaviour, const EInteractionEventType EventType, const FInputActionValue& Value)
{
	ActionBehaviourReplicationStart(Behaviour, EventType, Value);
}

void UUBaseInteractionComponent::MulticastActionBehaviourReplicationStartRpc_Implementation(
	UActionBehaviour* Behaviour, const EInteractionEventType EventType, const FInputActionValue& Value)
{
	Behaviour->OnActionEventEvent.Broadcast(this, EventType, Value);
}
