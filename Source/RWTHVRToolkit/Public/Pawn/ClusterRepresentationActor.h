﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ClusterRepresentationActor.generated.h"


class ARWTHVRPlayerState;
#if PLATFORM_SUPPORTS_CLUSTER
class ADisplayClusterRootActor;
#endif

UCLASS()
class RWTHVRTOOLKIT_API AClusterRepresentationActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AClusterRepresentationActor();

	virtual void BeginPlay() override;

	void AttachDCRAIfRequired(const ARWTHVRPlayerState* OptionalPlayerState = nullptr);

private:
	bool bIsAttached = false;

#if PLATFORM_SUPPORTS_CLUSTER
	bool AttachDCRA();
	ADisplayClusterRootActor* SpawnDCRA();

#endif
};