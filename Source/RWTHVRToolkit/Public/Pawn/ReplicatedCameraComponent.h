// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Core/ReplicatedTransform.h"
#include "ReplicatedCameraComponent.generated.h"

/**
 * Simple CameraComponent with added client-side transform replication.
 */
UCLASS()
class RWTHVRTOOLKIT_API UReplicatedCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UReplicatedCameraComponent();

protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Full transform update replication
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	*  See UClientTransformReplication for a description of the replication functions, they work exactly the same way.
	*/

	// Rate to update the position to the server, 100htz is default (same as replication rate, should also hit every tick).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Networking",
		meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

	// Accumulates time until next send
	float ControllerNetUpdateCount;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedTransform, Category = "Networking")
	FReplicatedTransform ReplicatedTransform;

	void UpdateState(float DeltaTime);

	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		//For now, directly apply the transforms:
		if (!GetOwner()->HasLocalNetOwner())
			SetRelativeLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerSendControllerTransformRpc(FReplicatedTransform NewTransform);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
