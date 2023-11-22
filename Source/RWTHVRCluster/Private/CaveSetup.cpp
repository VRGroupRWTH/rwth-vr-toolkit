// Fill out your copyright notice in the Description page of Project Settings.


#include "CaveSetup.h"

#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"


// Sets default values
ACaveSetup::ACaveSetup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);

	// Actor needs to replicate, as it is attached to the pawn on the server.
	bReplicates = true;
}

// Called when the game starts or when spawned
void ACaveSetup::BeginPlay()
{
	Super::BeginPlay();

	if (!URWTHVRUtilities::IsRoomMountedMode())
	{
		return;
	}

	// Spawn all actors that are set in the blueprint asset.
	for (const auto ActorClass : ActorsToSpawnOnCave)
	{
		if (const auto World = GetWorld())
		{
			const auto Actor = World->SpawnActor(ActorClass);
			Actor->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			UE_LOGFMT(LogTemp, Display, "CaveSetup: Spawned Actor {Actor} on the Cave and attached it.",
			          Actor->GetName());
		}
	}

	// Apply the DTrack LiveLink Preset. Only do this if we are the primaryNode

	if (URWTHVRUtilities::IsPrimaryNode())
	{
		if (LiveLinkPresetToApplyOnCave && LiveLinkPresetToApplyOnCave->IsValidLowLevelFast())
		{
			LiveLinkPresetToApplyOnCave->ApplyToClientLatent();

			UE_LOGFMT(LogTemp, Display, "CaveSetup: Applied LiveLinkPreset {Preset} to Client.",
			          LiveLinkPresetToApplyOnCave->GetName());
		}
	}
}
