// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkPreset.h"
#include "GameFramework/Actor.h"
#include "CaveSetup.generated.h"

/**
 * Simple Actor that needs to be added to the level which spawns Cave-related actors
 * such as the CaveOverlay.
 * It attaches itself to the Primary Node's Pawn and then replicates on the server.
 */

UCLASS(hideCategories = (Rendering, Input, Actor, Base, Collision, Shape, Physics, HLOD))
class RWTHVRCLUSTER_API ACaveSetup : public AActor
{
	GENERATED_BODY()

public:
	ACaveSetup();

	UPROPERTY(EditAnywhere)
	TArray<UClass*> ActorsToSpawnOnCave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULiveLinkPreset* LiveLinkPresetToApplyOnCave;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
