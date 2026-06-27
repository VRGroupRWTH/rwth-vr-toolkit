// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DisplayClusterSceneComponentSyncParent.h"
#include "VRClusterSyncComponent.generated.h"

/**
 * Syncs the CAVE pawn position across nDisplay cluster nodes.
 *
 * GetSyncTransform (primary): reads pawn world transform — this is the authoritative real-time position.
 * SetSyncTransform (secondary): writes directly to the CachedDCRA on the CRA, NOT to the pawn.
 *   Writing to the pawn is wrong on secondary nodes because Mover's SimProxy interpolator
 *   overrides SetWorldTransform every tick, causing the view to lag behind by the interpolation buffer.
 *   The DCRA drives the CAVE rendering, so setting it directly gives immediate, lag-free sync.
 */
UCLASS()
class RWTHVRTOOLKIT_API UVRClusterSyncComponent : public UDisplayClusterSceneComponentSyncParent
{
	GENERATED_BODY()

public:
	// Returns the last transform received via SetSyncTransform (secondary/node_floor only).
	// Returns false if SetSyncTransform has never been called.
	bool GetLastReceivedSyncTransform(FTransform& OutTransform) const;

	// Seconds since SetSyncTransform was last called. Returns a large value if never called.
	double GetSecondsSinceLastSyncTransform() const;

protected:
	virtual FString GenerateSyncId() override;
	virtual FTransform GetSyncTransform() const override;
	virtual void SetSyncTransform(const FTransform& t) override;
	virtual bool IsDirty() const override;
	virtual void ClearDirty() override;

private:
	mutable FVector LastWorldLoc = FVector::ZeroVector;
	mutable FRotator LastWorldRot = FRotator::ZeroRotator;
	mutable bool WasMovingLastFrame = false;

	// Stable snapshot used by GetSyncTransform to suppress autonomous-proxy reconciliation drift.
	mutable FVector StableSyncLoc = FVector::ZeroVector;
	mutable FRotator StableSyncRot = FRotator::ZeroRotator;
	mutable bool bHasStableSync = false;

	// Last transform received on secondary node via SetSyncTransform.
	FTransform LastReceivedSyncTransform = FTransform::Identity;
	double LastSyncTransformTimeSeconds = -1e9;
	bool bHasReceivedSyncTransform = false;
};
