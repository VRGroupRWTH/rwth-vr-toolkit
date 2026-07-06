// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VRClusterSyncComponent.h"
#include "RWTHVRToolkit.h"
#include "Pawn/ClusterRepresentationActor.h"
#include "Utility/RWTHVRUtilities.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "DisplayClusterRootActor.h"
#endif

FString UVRClusterSyncComponent::GenerateSyncId()
{
	return FString::Printf(TEXT("VR_%s"), *GetOwner()->GetFullName());
}

FTransform UVRClusterSyncComponent::GetSyncTransform() const
{
	if (!GetOwner()) return FTransform::Identity;
	return GetOwner()->GetActorTransform();
}

#if PLATFORM_SUPPORTS_CLUSTER
// Finds the DCRA driven by the CRA attached to this pawn. Returns nullptr on the primary node
// path, or on secondary before the CRA has cached its DCRA.
static ADisplayClusterRootActor* FindAttachedDCRA(const AActor* Owner)
{
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn) return nullptr;

	TArray<AActor*> AttachedActors;
	OwnerPawn->GetAttachedActors(AttachedActors);
	for (AActor* Attached : AttachedActors)
	{
		if (const AClusterRepresentationActor* CRA = Cast<AClusterRepresentationActor>(Attached))
		{
			return CRA->GetCachedDCRA();
		}
	}
	return nullptr;
}
#endif

void UVRClusterSyncComponent::SetSyncTransform(const FTransform& t)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	USceneComponent* Root = Owner->GetRootComponent();

#if PLATFORM_SUPPORTS_CLUSTER
	// On secondary nodes the pawn is ROLE_SimulatedProxy. Mover's interpolator overrides the
	// pawn root every tick, so writing to the pawn alone lags the view by the interpolation
	// buffer. The DCRA is what drives CAVE rendering, so we drive it directly (bypassing Mover)
	// and also snap the pawn root — Mover's UpdatedComponent is redirected to a dummy on this
	// node (AVRPawn), so this write is the sole driver of the visible pawn.
	if (ADisplayClusterRootActor* DCRA = FindAttachedDCRA(Owner))
	{
		DCRA->SetActorLocationAndRotation(t.GetLocation(), t.GetRotation(), false, nullptr,
										  ETeleportType::TeleportPhysics);
		if (Root)
		{
			Root->SetWorldLocationAndRotation(t.GetLocation(), t.GetRotation(), false, nullptr,
											  ETeleportType::TeleportPhysics);
		}
		return;
	}
#endif

	// Fallback: no CRA/DCRA found (primary node, or secondary before the CRA has attached).
	if (Root)
	{
		Root->SetWorldTransform(t, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

bool UVRClusterSyncComponent::IsDirty() const
{
	if (!GetOwner()) return false;
	const FVector Current = GetOwner()->GetActorLocation();

	const bool bChanged = !Current.Equals(LastWorldLoc, 1.0f);

	LastWorldLoc = Current; // update every frame so we track velocity, not cumulative offset
	return bChanged;
}

void UVRClusterSyncComponent::ClearDirty()
{
	// Intentionally a no-op: LastWorldLoc is updated every frame in IsDirty(), which is what
	// the velocity-based dirty check relies on. Nothing to reset here.
}
