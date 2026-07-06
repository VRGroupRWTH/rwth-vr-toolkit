// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ClusterRepresentationActor.h"

#include "Core/RWTHVRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "DisplayClusterRootActor.h"
#include "IDisplayCluster.h"
#include "Config/IDisplayClusterConfigManager.h"
#include "Game/IDisplayClusterGameManager.h"
#endif

// Sets default values
AClusterRepresentationActor::AClusterRepresentationActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetActorEnableCollision(false);

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Origin")));
}

void AClusterRepresentationActor::BeginPlay()
{
	Super::BeginPlay();
	// will fail if we're in replicated mode and PlayerState has not yet replicated fully
	// Therefore we also execute this
	AttachDCRAIfRequired();
}

void AClusterRepresentationActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if PLATFORM_SUPPORTS_CLUSTER
	// Only the primary node drives the DCRA from CRA tick.
	// Secondary nodes have their DCRA driven directly by VRClusterSyncComponent::SetSyncTransform,
	// which bypasses Mover's SimProxy interpolation. If we also pushed from the CRA tick on
	// secondary, we'd fight SetSyncTransform with the lagging pawn/CRA position.
	if (!URWTHVRUtilities::IsPrimaryNode()) return;
	if (!CachedDCRA.IsValid()) return;

	const FVector NewLoc = GetActorLocation();
	const FRotator NewRot = GetActorRotation();

	const float PosDeltaSq = FVector::DistSquared(NewLoc, LastAppliedDCRALoc);
	const float RotDelta = FMath::Abs(FRotator::NormalizeAxis(NewRot.Yaw - LastAppliedDCRARot.Yaw))
		+ FMath::Abs(FRotator::NormalizeAxis(NewRot.Pitch - LastAppliedDCRARot.Pitch))
		+ FMath::Abs(FRotator::NormalizeAxis(NewRot.Roll - LastAppliedDCRARot.Roll));

	const bool bPositionChanged = PosDeltaSq > FMath::Square(PositionDeadzoneCm);
	const bool bRotationChanged = RotDelta > RotationDeadzoneDeg;

	if (bPositionChanged || bRotationChanged)
	{
		CachedDCRA->SetActorLocationAndRotation(NewLoc, NewRot, false, nullptr, ETeleportType::TeleportPhysics);
		LastAppliedDCRALoc = NewLoc;
		LastAppliedDCRARot = NewRot;
	}
#endif
}

void AClusterRepresentationActor::AttachDCRAIfRequired(const ARWTHVRPlayerState* OptionalPlayerState)
{
	// We need to identify the correct ClusterRepresentationActor to do the attachment.
	// 1. Either we are the local net owner -> Primary Node Pawn
	// This is hard to do as things might not be synchronized yet.
	// In this case I think this is save to do because either:
	// - We are in standalone mode, then attach it for all nodes
	// - If we are a client, this actor has been spawned on the server only. Therefore I assume that if we
	//   have replicated this actor to our client, we're good to go.

#if PLATFORM_SUPPORTS_CLUSTER

	if (!URWTHVRUtilities::IsRoomMountedMode())
		return;

	if (bIsAttached)
	{
		UE_LOGFMT(Toolkit, Display, "{Name} AttachDCRAIfRequired: Already attached, skipping repeated attachment.",
				  GetName());
		return;
	}

	UE_LOGFMT(Toolkit, Display, "{Name} AttachDCRAIfRequired: Starting DCRA Attachment process.", GetName());

	// This should give us the first local player controller
	const auto* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Only run this for the local controller - redundant, but double check
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		UE_LOGFMT(Toolkit, Warning,
				  "{Name} AttachDCRAIfRequired: PlayerController not valid or not locally controlled.", GetName());
		return;
	}
	const auto* PlayerState =
		OptionalPlayerState != nullptr ? OptionalPlayerState : PlayerController->GetPlayerState<ARWTHVRPlayerState>();
	if (!PlayerState)
	{
		UE_LOGFMT(Toolkit, Error,
				  "{Name} AttachDCRAIfRequired: PlayerState is not valid or not of type "
				  "ARWTHVRPlayerState.",
				  GetName());
		return;
	}

	const auto CCA = PlayerState->GetCorrespondingClusterActor();

	if (CCA == nullptr) // this can happen often if property isn't replicated yet, this is okay.
		return;

	UE_LOGFMT(Toolkit, Display,
			  "{Name} AttachDCRAIfRequired: Player State is {PlayerState}, PlayerState->CCA is {CCA}.", GetName(),
			  PlayerState->GetName(), CCA->GetName());

	// The local player this is executed on corresponds to this actor
	if (CCA == this)
	{
		UE_LOGFMT(Toolkit, Display, "{Name} AttachDCRAIfRequired: Attaching DCRA to {Name}.", GetName(),
				  CCA->GetName());

		bIsAttached = AttachDCRA();
	}
#endif
}

#if PLATFORM_SUPPORTS_CLUSTER
bool AClusterRepresentationActor::AttachDCRA()
{
	if (URWTHVRUtilities::IsRoomMountedMode())
	{
		UE_LOGFMT(Toolkit, Display, "{Name}: Trying to attach DCRA", GetName());

		auto DCRA = IDisplayCluster::Get().GetGameMgr()->GetRootActor();

		if (!IsValid(DCRA))
		{
			UE_LOGFMT(Toolkit, Display, "No Valid DCRA in BeginPlay. Spawning manually.");

			DCRA = SpawnDCRA();
			if (!IsValid(DCRA))
			{
				UE_LOGFMT(Toolkit, Error, "Failed to spawn correct DCRA, cannot attach it to ClusterRepresentation.");
				return false;
			}
		}
		else // if we just spawned the DCRA, it is not yet the primary one and this check makes no sense
		{
			if (!DCRA->IsPrimaryRootActor())
			{
				UE_LOGFMT(Toolkit, Error, "Found DCRA {Name} is not the primary DCRA of Cluster with Id!",
						  DCRA->GetName());
				return false;
			}
		}

		// Do NOT use AttachToActor — a hard attachment mirrors every sub-mm wiggle from Mover's
		// idle solver noise directly into the DCRA, feeding node_floor's SimProxy interpolator
		// indefinitely. Instead, cache the DCRA and drive it manually from Tick() with a deadzone.
		CachedDCRA = DCRA;
		const FVector SnapLoc = GetActorLocation();
		const FRotator SnapRot = GetActorRotation();
		DCRA->SetActorLocationAndRotation(SnapLoc, SnapRot, false, nullptr, ETeleportType::TeleportPhysics);
		LastAppliedDCRALoc = SnapLoc;
		LastAppliedDCRARot = SnapRot;

		DCRA->SetActorEnableCollision(false);
	}
	return true;
}

ADisplayClusterRootActor* AClusterRepresentationActor::SpawnDCRA()
{
	UDisplayClusterConfigurationData* ConfigData = IDisplayCluster::Get().GetConfigMgr()->GetConfig();


	// Function similar to the one from DisplayClusterGameManager.
	ADisplayClusterRootActor* RootActor = nullptr;
	// We need to use generated class as it's the only one available in packaged buidls
	const FString AssetPath =
		(ConfigData->Info.AssetPath.EndsWith(TEXT("_C")) ? ConfigData->Info.AssetPath
														 : ConfigData->Info.AssetPath + TEXT("_C"));

	if (UClass* ActorClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *AssetPath)))
	{
		// Spawn the actor
		if (AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass))
		{
			RootActor = Cast<ADisplayClusterRootActor>(NewActor);

			if (RootActor)
			{
				UE_LOG(Toolkit, Log, TEXT("Instantiated DCRA from asset '%s'"), *ConfigData->Info.AssetPath);

				// Override actor settings in case the config file contains some updates
				RootActor->OverrideFromConfig(ConfigData);
			}
		}
	}
	return RootActor;
}

#endif
