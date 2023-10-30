// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/VRTransformRep.h"
#include "ClientTransformReplication.generated.h"


/*
* Simple Client Transform Replication Component. Replicates the owning actor's root transform from owning client to server,
* from there to all other clients.
*/
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UClientTransformReplication : public UActorComponent
{
	GENERATED_BODY()

public:
	UClientTransformReplication();

protected:
	/*
	* For now, replicate in a naive sending every x ms if the transform has changed.
	* This is way overkill, as we should only be sending input. However, I am not yet fully sure how
	* the Unreal Client-Authoritative thingy works and what part simulates e.g. gravity.
	* As this modifies only the tracking origin, latency should not be that much of an issue, so theoretically
	* Server-Authority should work here too, in which case we'd just send the x and y input.
	* Try both ways.
	*/

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Full transform update replication
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	// Rate to update the position to the server, 100htz is default (same as replication rate, should also hit every tick).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Networking",
		meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

	// Accumulates time until next send
	float ControllerNetUpdateCount;

	// Replicated transform property - used to replicate from server to all non-owning clients
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedTransform, Category = "Networking")
	FVRTransformRep ReplicatedTransform;

	// Called whenever ReplicatedTransform is replicated to clients. Not called on Server/Owning client
	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		// Modify owner position - how does this work in movement components?
		// For now, directly apply the transforms:
		auto* OwningActor = GetOwner();
		if (OwningActor && OwningActor->HasValidRootComponent())
			OwningActor->SetActorLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	// Unreliable Server RPC that sends the transform from owning client to the server
	UFUNCTION(Unreliable, Server, WithValidation)
	void SendControllerTransform_ServerRpc(FVRTransformRep NewTransform);

	void UpdateState(float DeltaTime);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
