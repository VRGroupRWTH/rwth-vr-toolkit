// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ClusterRepresentationActor.generated.h"


class ADisplayClusterRootActor;
UCLASS()
class RWTHVRTOOLKIT_API AClusterRepresentationActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AClusterRepresentationActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ClusterId = -1;

	virtual void BeginPlay() override;
	

private:
	bool AttachDCRA();
	ADisplayClusterRootActor* SpawnDCRA();
};
