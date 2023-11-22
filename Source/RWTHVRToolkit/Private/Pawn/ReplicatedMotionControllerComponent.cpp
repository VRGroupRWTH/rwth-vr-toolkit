// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ReplicatedMotionControllerComponent.h"

#include "Net/UnrealNetwork.h"

UReplicatedMotionControllerComponent::UReplicatedMotionControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetIsReplicatedByDefault(true);

	// Direct transform replication
	ControllerNetUpdateRate = 100.0f; // 100 htz is default
	ControllerNetUpdateCount = 0.0f;
}

// See UClientTransformReplication::UpdateState
void UReplicatedMotionControllerComponent::UpdateState(float DeltaTime)
{
	if (GetOwner()->HasLocalNetOwner())
	{
		if (GetIsReplicated())
		{
			const FVector Loc = GetRelativeLocation();
			const FRotator Rot = GetRelativeRotation();

			if (!Loc.Equals(ReplicatedTransform.Position) || !Rot.Equals(ReplicatedTransform.Rotation))
			{
				ControllerNetUpdateCount += DeltaTime;
				if (ControllerNetUpdateCount >= (1.0f / ControllerNetUpdateRate)) // todo save inverse?
				{
					ControllerNetUpdateCount = 0.0f;

					ReplicatedTransform.Position = Loc;
					ReplicatedTransform.Rotation = Rot;
					if (GetNetMode() == NM_Client)
					{
						ServerSendControllerTransformRpc(ReplicatedTransform);
					}
				}
			}
		}
	}
}

void UReplicatedMotionControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateState(DeltaTime);
}

void UReplicatedMotionControllerComponent::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable the basic built in replication of relative transform data, as we are using our own transform sync/state update
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	// Skipping the owner with this as the owner will use the controllers location directly
	DOREPLIFETIME_CONDITION(UReplicatedMotionControllerComponent, ReplicatedTransform, COND_SkipOwner);
	DOREPLIFETIME(UReplicatedMotionControllerComponent, ControllerNetUpdateRate);
}

// See UClientTransformReplication::ServerSendControllerTransformRpc_Implementation
void UReplicatedMotionControllerComponent::ServerSendControllerTransformRpc_Implementation(
	FReplicatedTransform NewTransform)
{
	// Store new transform and trigger OnRep_Function
	ReplicatedTransform = NewTransform;

	if (!GetOwner()->HasLocalNetOwner())
		OnRep_ReplicatedTransform();
}

bool UReplicatedMotionControllerComponent::ServerSendControllerTransformRpc_Validate(FReplicatedTransform NewTransform)
{
	return true;
	// Optionally check to make sure that player is inside of their bounds and deny it if they aren't?
}
