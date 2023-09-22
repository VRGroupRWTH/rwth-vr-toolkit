// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ClientTransformReplication.h"

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
	const auto* Pawn = GetOwner();
	if (Pawn && Pawn->HasLocalNetOwner())
	{
		if (GetIsReplicated())
		{
			const FVector Loc = Pawn->GetActorLocation();
			const FRotator Rot = Pawn->GetActorRotation();

			if (!Loc.Equals(ReplicatedTransform.Position) || !Rot.Equals(ReplicatedTransform.Rotation))
			{
				ControllerNetUpdateCount += DeltaTime;
				if (ControllerNetUpdateCount >= (1.0f / ControllerNetUpdateRate)) // todo save inverse?
					{
					ControllerNetUpdateCount = 0.0f;

					ReplicatedTransform.Position = Loc;
					ReplicatedTransform.Rotation = Rot;
					if (GetNetMode() == NM_Client) // why do we differentiate here between netmode and authority?
						{
						SendControllerTransform_ServerRpc(ReplicatedTransform);
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
	UpdateState(DeltaTime);
}

void UClientTransformReplication::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Skipping the owner with this as the owner will use the controllers location directly
	DOREPLIFETIME_CONDITION(UClientTransformReplication, ReplicatedTransform, COND_SkipOwner);
	DOREPLIFETIME(UClientTransformReplication, ControllerNetUpdateRate);
}

void UClientTransformReplication::SendControllerTransform_ServerRpc_Implementation(FVRTransformRep NewTransform)
{
	// Store new transform and trigger OnRep_Function
	ReplicatedTransform = NewTransform;

	if (!GetOwner()->HasLocalNetOwner())
		OnRep_ReplicatedTransform();
}

bool UClientTransformReplication::SendControllerTransform_ServerRpc_Validate(FVRTransformRep NewTransform)
{
	return true;
	// Optionally check to make sure that player is inside of their bounds and deny it if they aren't?
}