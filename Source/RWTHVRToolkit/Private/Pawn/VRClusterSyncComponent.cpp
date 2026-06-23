// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/VRClusterSyncComponent.h"

FString UVRClusterSyncComponent::GenerateSyncId()
{
	return FString::Printf(TEXT("VR_%s"), *GetOwner()->GetFullName());
}

FTransform UVRClusterSyncComponent::GetSyncTransform() const
{
	if (!GetOwner()) return FTransform::Identity;
	// Sync actor world transform
	return GetOwner()->GetActorTransform();
}

void UVRClusterSyncComponent::SetSyncTransform(const FTransform& t)
{
	if (!GetOwner()) return;
	USceneComponent* Root = GetOwner()->GetRootComponent();
	if (Root)
	{
		FVector CurrentLoc = Root->GetComponentLocation();
		FVector NewLoc = t.GetLocation();

		UE_LOG(LogTemp, Warning,
			TEXT("[SYNC-SET-CHECK] Applying transform | NewLoc:%s | OwnerLoc:%s"),
			*t.GetLocation().ToString(),
			GetOwner() ? *GetOwner()->GetActorLocation().ToString() : TEXT("no owner"));

		Root->SetWorldTransform(t, false, nullptr,
			ETeleportType::TeleportPhysics);
	}
}

bool UVRClusterSyncComponent::IsDirty() const
{
	if (!GetOwner()) return false;
	FVector Current = GetOwner()->GetActorLocation();
	bool bChanged = !Current.Equals(LastWorldLoc, 0.1f);
    
	if (!bChanged && WasMovingLastFrame)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[DIRTY] Just stopped — IsDirty:false | Loc:%s"),
			*Current.ToString()
		);
	}
	WasMovingLastFrame = bChanged;
	return bChanged;
}

void UVRClusterSyncComponent::ClearDirty()
{
	if (!GetOwner()) return;
	LastWorldLoc = GetOwner()->GetActorLocation();
	LastWorldRot = GetOwner()->GetActorRotation();
}
