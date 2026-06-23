// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DisplayClusterSceneComponentSyncParent.h"
#include "VRClusterSyncComponent.generated.h"

/**
 * 
 */
UCLASS()
class RWTHVRTOOLKIT_API UVRClusterSyncComponent : public UDisplayClusterSceneComponentSyncParent
{
	GENERATED_BODY()
	
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
};
