// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/VRClusterSyncComponent.h"
#include "RWTHVRToolkit.h"
#include "Pawn/ClusterRepresentationActor.h"
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

	const FVector CurrentLoc = GetOwner()->GetActorLocation();
	const FRotator CurrentRot = GetOwner()->GetActorRotation();

	// Deadzone: suppress Mover autonomous-proxy reconciliation drift.
	// After the player stops, Mover smoothly corrects the client position toward the server
	// authoritative value. That drift (~25 units/frame for ~2s) appears as fake continued
	// motion on secondary nodes. We latch the position once movement falls below 1cm/frame
	// and only update the latch when real movement exceeds that threshold.
	constexpr float PositionDeadzoneCm = 1.0f;
	constexpr float RotationDeadzoneDeg = 0.5f;

	if (!bHasStableSync)
	{
		StableSyncLoc = CurrentLoc;
		StableSyncRot = CurrentRot;
		bHasStableSync = true;
	}

	const float PosDelta = FVector::Dist(CurrentLoc, StableSyncLoc);
	const float RotDelta = FMath::Abs(FRotator::NormalizeAxis(CurrentRot.Yaw - StableSyncRot.Yaw))
		+ FMath::Abs(FRotator::NormalizeAxis(CurrentRot.Pitch - StableSyncRot.Pitch))
		+ FMath::Abs(FRotator::NormalizeAxis(CurrentRot.Roll - StableSyncRot.Roll));

	if (PosDelta > PositionDeadzoneCm || RotDelta > RotationDeadzoneDeg)
	{
		StableSyncLoc = CurrentLoc;
		StableSyncRot = CurrentRot;
	}

	return FTransform(StableSyncRot, StableSyncLoc, GetOwner()->GetActorScale3D());
}

bool UVRClusterSyncComponent::GetLastReceivedSyncTransform(FTransform& OutTransform) const
{
	if (!bHasReceivedSyncTransform) return false;
	OutTransform = LastReceivedSyncTransform;
	return true;
}

double UVRClusterSyncComponent::GetSecondsSinceLastSyncTransform() const
{
	if (!bHasReceivedSyncTransform) return 1e9;
	return FPlatformTime::Seconds() - LastSyncTransformTimeSeconds;
}

void UVRClusterSyncComponent::SetSyncTransform(const FTransform& t)
{
	if (!GetOwner()) return;

	// Record timestamp and last received transform for SimProxy snap logic in VRMoverComponent.
	LastReceivedSyncTransform = t;
	LastSyncTransformTimeSeconds = FPlatformTime::Seconds();
	bHasReceivedSyncTransform = true;

#if PLATFORM_SUPPORTS_CLUSTER
	// On secondary nodes, the pawn is ROLE_SimulatedProxy. Mover's interpolation system
	// overrides SetWorldTransform on the pawn root every tick, so writing to the pawn
	// produces no visible effect — the interpolator wins and the view lags by the buffer.
	//
	// The DCRA is what drives the CAVE rendering on this node. We go directly to it
	// via the CRA that is attached to the pawn, bypassing Mover entirely.
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		// Find the CRA attached to this pawn and drive the DCRA directly
		TArray<AActor*> AttachedActors;
		OwnerPawn->GetAttachedActors(AttachedActors);
		for (AActor* Attached : AttachedActors)
		{
			if (AClusterRepresentationActor* CRA = Cast<AClusterRepresentationActor>(Attached))
			{
				ADisplayClusterRootActor* DCRA = CRA->GetCachedDCRA();
				if (DCRA)
				{
#if CAVE_SYNC_DEBUG_LOGS // [SYNC-SET-CHECK] debug: transform applied to DCRA/pawn root on secondary node
					UE_LOG(LogTemp, Warning,
						TEXT("[SYNC-SET-CHECK] Pawn:%s | Applying transform | NewLoc:%s | OwnerLoc:%s"),
						*GetOwner()->GetName(),
						*t.GetLocation().ToCompactString(),
						*GetOwner()->GetActorLocation().ToCompactString());
#endif

					// Drive DCRA directly — bypasses Mover SimProxy interpolation for rendering.
					DCRA->SetActorLocationAndRotation(
						t.GetLocation(), t.GetRotation(),
						false, nullptr, ETeleportType::TeleportPhysics);

					// Also snap the pawn root so the avatar mesh sits at the correct world
					// position. On secondary nodes Mover's UpdatedComponent is redirected to a
					// dummy (AVRPawn::BeginPlay), so this write is the sole driver of the pawn.
					USceneComponent* Root = GetOwner()->GetRootComponent();
					if (Root)
					{
						Root->SetWorldLocationAndRotation(
							t.GetLocation(), t.GetRotation(),
							false, nullptr, ETeleportType::TeleportPhysics);
					}

					return;
				}
			}
		}
	}
#endif

	// Fallback: no CRA/DCRA found — write directly to the pawn root (primary node path,
	// or secondary before CRA has attached).
	USceneComponent* Root = GetOwner()->GetRootComponent();
	if (Root)
	{
#if CAVE_SYNC_DEBUG_LOGS // [SYNC-SET-CHECK] debug: fallback path when no CRA/DCRA is attached yet
		UE_LOG(LogTemp, Warning,
			TEXT("[SYNC-SET-CHECK] Pawn:%s | Fallback — setting pawn root | NewLoc:%s | OwnerLoc:%s"),
			*GetOwner()->GetName(),
			*t.GetLocation().ToCompactString(),
			*GetOwner()->GetActorLocation().ToCompactString());
#endif

		Root->SetWorldTransform(t, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

bool UVRClusterSyncComponent::IsDirty() const
{
	if (!GetOwner()) return false;
	const FVector Current = GetOwner()->GetActorLocation();

	// Compare frame-to-frame delta, not cumulative drift from last ClearDirty.
	// Mover's autonomous-proxy reconciliation moves the pawn ~25 units/frame for ~2s after
	// stopping — that's large enough to fool a cumulative check. A per-frame velocity check
	// catches only genuine intentional movement (joystick or room-scale).
	const bool bChanged = !Current.Equals(LastWorldLoc, 1.0f);

#if CAVE_SYNC_DEBUG_LOGS // [DIRTY] debug: logs the frame the primary node detects movement stop
	if (!bChanged && WasMovingLastFrame)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[DIRTY] Pawn:%s | Just stopped — IsDirty:false | Loc:%s"),
			*GetOwner()->GetName(),
			*Current.ToCompactString());
	}
#endif
	WasMovingLastFrame = bChanged;
	LastWorldLoc = Current; // update every frame so we track velocity, not cumulative offset
	return bChanged;
}

void UVRClusterSyncComponent::ClearDirty()
{
	// Intentionally left minimal — LastWorldLoc is now updated every frame in IsDirty.
	if (!GetOwner()) return;
	LastWorldRot = GetOwner()->GetActorRotation();
}
