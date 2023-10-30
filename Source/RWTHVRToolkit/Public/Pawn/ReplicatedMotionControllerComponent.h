// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Core/VRTransformRep.h"
#include "ReplicatedMotionControllerComponent.generated.h"

/**
 * Simple MotionControllerComponent with added client-side transform replication.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UReplicatedMotionControllerComponent : public UMotionControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UReplicatedMotionControllerComponent();

protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Full transform update replication
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	// Rate to update the position to the server, 100htz is default (same as replication rate, should also hit every tick).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Networking",
		meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

	// Accumulates time until next send
	float ControllerNetUpdateCount;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedTransform, Category = "Networking")
	FVRTransformRep ReplicatedTransform;

	void UpdateState(float DeltaTime);

	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		// For now, directly apply the transforms:
		SetRelativeLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerSendControllerTransformRpc(FVRTransformRep NewTransform);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
