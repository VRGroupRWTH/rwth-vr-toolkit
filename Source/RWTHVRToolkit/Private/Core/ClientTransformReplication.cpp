// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/ClientTransformReplication.h"

#include "Net/UnrealNetwork.h"

UClientTransformReplication::UClientTransformReplication()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetIsReplicatedByDefault(true);

	// Direct transform replication
	ControllerNetUpdateRate = 100.0f; // 100 htz is default
	ControllerNetUpdateCount = 0.0f;
}

// Naive direct transform replication (replace with input rep?)
void UClientTransformReplication::UpdateState(float DeltaTime)
{
	// We only perform the state update on the local owner of the actor. This is our (client-)authority
	const auto* OwningActor = GetOwner();
	if (OwningActor && OwningActor->HasLocalNetOwner())
	{
		// Only do this if we actually replicate the actor
		if (GetIsReplicated())
		{
			const FVector Loc = OwningActor->GetActorLocation();
			const FRotator Rot = OwningActor->GetActorRotation();

			// Only update state if the local state changed
			if (!Loc.Equals(ReplicatedTransform.Position) || !Rot.Equals(ReplicatedTransform.Rotation))
			{
				// Factor in NetUpdateRate
				ControllerNetUpdateCount += DeltaTime;
				if (ControllerNetUpdateCount >= (1.0f / ControllerNetUpdateRate))
				{
					ControllerNetUpdateCount = 0.0f;

					// The local state has changed and we're within the update rate. Apply the new local state to the
					// replicated variable - this just saves the new local state on the local net owner. 
					ReplicatedTransform.Position = Loc;
					ReplicatedTransform.Rotation = Rot;

					// If we are running as a client, push the state to the server by calling the respective RPC.
					// This is required in case we are both the server and the local net owner, in which case simply
					// updating the local state is enough.
					if (GetNetMode() == NM_Client)
					{
						ServerSendControllerTransformRpc(ReplicatedTransform);
					}
				}
			}
		}
	}
}

void UClientTransformReplication::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Perform a state update (if required) each tick. A state update means that we check whether
	// our transform changed. If it did, send the new one to the server if we're within our NetUpdateRate.
	UpdateState(DeltaTime);
}

// this function just defines how our properties are replicated
void UClientTransformReplication::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Skipping the owner with this as the owner will use the local state directly
	DOREPLIFETIME_CONDITION(UClientTransformReplication, ReplicatedTransform, COND_SkipOwner);
	DOREPLIFETIME(UClientTransformReplication, ControllerNetUpdateRate);
}

// Apply the state update on the server
void UClientTransformReplication::ServerSendControllerTransformRpc_Implementation(FVRTransformRep NewTransform)
{
	// Store new transform and trigger OnRep_Function
	ReplicatedTransform = NewTransform;

	// We are on the server here. If we are additionally the local net owner, i.e. the source of the stateupdate,
	// it means we already directly apply our transform, as we are either a listen server or running in standalone.
	// Therefore only apply the On_Rep function if we are not the local net owner.
	// The OnRep function is doing the actual state update.
	if (!GetOwner()->HasLocalNetOwner())
		OnRep_ReplicatedTransform();
}

bool UClientTransformReplication::ServerSendControllerTransformRpc_Validate(FVRTransformRep NewTransform)
{
	return true;
	// Optionally check to make sure that player is inside of their bounds and deny it if they aren't?
}
