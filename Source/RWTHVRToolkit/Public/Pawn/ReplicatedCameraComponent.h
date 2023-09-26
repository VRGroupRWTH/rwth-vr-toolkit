﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "ReplicatedCameraComponent.generated.h"

/**
 * 
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
		//For now, directly apply the transforms:
		if (!GetOwner()->HasLocalNetOwner())
			SetRelativeLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
	void SendControllerTransform_ServerRpc(FVRTransformRep NewTransform);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
};
