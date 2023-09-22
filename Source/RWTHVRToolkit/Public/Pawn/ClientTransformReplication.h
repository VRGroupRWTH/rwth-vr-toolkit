// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "ClientTransformReplication.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UClientTransformReplication : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Networking", meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

	// Accumulates time until next send
	float ControllerNetUpdateCount;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedTransform, Category = "Networking")
	FVRTransformRep ReplicatedTransform;
	
	void UpdateState(float DeltaTime);

	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		// Modify pawn position - how does this work in movement components?
		// For now, directly apply the transforms:
		auto* Pawn = GetOwner();
		if (Pawn && Pawn->HasValidRootComponent())
			Pawn->SetActorLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
	void SendControllerTransform_ServerRpc(FVRTransformRep NewTransform);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
