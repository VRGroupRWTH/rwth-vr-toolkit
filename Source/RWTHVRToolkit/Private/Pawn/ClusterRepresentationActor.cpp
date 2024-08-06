// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ClusterRepresentationActor.h"

#include "DisplayClusterRootActor.h"
#include "IDisplayCluster.h"
#include "Config/IDisplayClusterConfigManager.h"
#include "Core/RWTHVRPlayerState.h"
#include "Game/IDisplayClusterGameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Utility/RWTHVRUtilities.h"

// Sets default values
AClusterRepresentationActor::AClusterRepresentationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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

void AClusterRepresentationActor::AttachDCRAIfRequired(const ARWTHVRPlayerState* OptionalPlayerState)
{
	// We need to identify the correct ClusterRepresentationActor to do the attachment.
	// 1. Either we are the local net owner -> Primary Node Pawn
	// This is hard to do as things might not be synchronized yet.
	// In this case I think this is save to do because either:
	// - We are in standalone mode, then attach it for all nodes
	// - If we are a client, this actor has been spawned on the server only. Therefore I assume that if we
	//   have replicated this actor to our client, we're good to go.

	if (!URWTHVRUtilities::IsRoomMountedMode())
		return;

	if (bIsAttached)
	{
		UE_LOGFMT(Toolkit, Display,
				  "AClusterRepresentationActor::AttachDCRAIfRequired: Already attached, skipping repeated attachment.");
		return;
	}
	
	UE_LOGFMT(Toolkit, Display, "AClusterRepresentationActor::AttachDCRAIfRequired: Starting DCRA Attachment process.");

	// This should give us the first local player controller
	const auto* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Only run this for the local controller - redundant, but double check
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		UE_LOGFMT(
			Toolkit, Warning,
			"AClusterRepresentationActor::AttachDCRAIfRequired: PlayerController not valid or not locally controlled.");
		return;
	}
	const auto* PlayerState =
		OptionalPlayerState != nullptr ? OptionalPlayerState : PlayerController->GetPlayerState<ARWTHVRPlayerState>();
	if (!PlayerState)
	{
		UE_LOGFMT(Toolkit, Error,
				  "AClusterRepresentationActor::AttachDCRAIfRequired: PlayerState is not valid or not of type "
				  "ARWTHVRPlayerState.");
		return;
	}

	// The local player this is executed on corresponds to this actor
	if (PlayerState->GetCorrespondingClusterActor() == this)
	{
		UE_LOGFMT(Toolkit, Display, "AClusterRepresentationActor::AttachDCRAIfRequired: Attaching DCRA to {Name}.",
				  GetName());

		bIsAttached = AttachDCRA();
	}
}

bool AClusterRepresentationActor::AttachDCRA()
{

#if PLATFORM_SUPPORTS_CLUSTER
	// Add an nDisplay Parent Sync Component. It syncs the parent's transform from master to clients.
	// This is required because for collision based movement, it can happen that the physics engine
	// for some reason acts different on the nodes, therefore leading to a potential desync when
	// e.g. colliding with an object while moving.

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

		bool bAttached = DCRA->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		UE_LOGFMT(Toolkit, Display, "Attaching {This} to DCRA {DCRA} returned {Res}", GetName(), DCRA->GetName(),
				  bAttached);

		DCRA->SetActorEnableCollision(false);
	}
#endif
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