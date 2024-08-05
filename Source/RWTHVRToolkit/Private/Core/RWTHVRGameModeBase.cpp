// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRGameModeBase.h"

#include "Core/RWTHVRPlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Pawn/ClusterRepresentationActor.h"
#include "Utility/RWTHVRUtilities.h"


ARWTHVRGameModeBase::ARWTHVRGameModeBase(const FObjectInitializer& ObjectInitializer)
{
	PlayerStateClass = ARWTHVRPlayerState::StaticClass();
}

FString ARWTHVRGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
										   const FString& Options, const FString& Portal)
{
	// Used by the DisplayClusterNetDriver/Connection to handshake nodes. Could use their types directly
	// but I don't really want to introduce a hard dependency here.
	const FString NodeNameKey = "node";
	const FString PrimaryNodeIdKey = "PrimaryNodeId";
	const FString ClusterIdKey = "ClusterId";

	// Check if we're using our custom PlayerState so that we can save the player type there.
	// If not, just ingore all related args.
	ARWTHVRPlayerState* State = Cast<ARWTHVRPlayerState>(NewPlayerController->PlayerState);

	if (State != nullptr)
	{
		int32 ClusterId = -1;
		if (UGameplayStatics::HasOption(Options, PrimaryNodeIdKey))
		{
			const FString PrimaryNodeId = UGameplayStatics::ParseOption(Options, PrimaryNodeIdKey);

			// When the primary node is a listen server, it apparently doesn't get the node option...
			// Could additionally check for listen, but this should be save enough.
			const FString NodeName = UGameplayStatics::HasOption(Options, NodeNameKey)
				? UGameplayStatics::ParseOption(Options, NodeNameKey)
				: PrimaryNodeId;

			ClusterId = UGameplayStatics::HasOption(Options, ClusterIdKey)
				? FCString::Atoi(*UGameplayStatics::ParseOption(Options, ClusterIdKey))
				: -1;

			const EPlayerType Type =
				NodeName == PrimaryNodeId ? EPlayerType::nDisplayPrimary : EPlayerType::nDisplaySecondary;
			State->RequestSetPlayerType(Type);
		}
		else if (GetNetMode() == NM_Standalone && URWTHVRUtilities::IsRoomMountedMode())
		{
			ClusterId = 0;
		}

		// If we're in none-standalone netmode, this is only executed on the server, as the GM only exists there.
		// On standalone, this is executed on every node.

		State->SetCorrespondingClusterId(ClusterId);

		auto* ClusterRepresentation = ConnectedClusters.Find(ClusterId);
		if (!ClusterRepresentation)
		{
			// No actor there yet, spawn it
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Name = FName(*FString::Printf(TEXT("ClusterRepresentation_%d"), ClusterId));
			ClusterRepresentation = GetWorld()->SpawnActor<AClusterRepresentationActor>(SpawnParameters);
			ClusterRepresentation->ClusterId = ClusterId;
			UE_LOGFMT(Toolkit, Display,
					  "ARWTHVRGameModeBase: Spawned ClusterRepresentationActor {Name} for Cluster {Id}",
					  ClusterRepresentation->GetName(), ClusterId);
		}

		UE_LOGFMT(Toolkit, Display, "ARWTHVRGameModeBase: Using ClusterRepresentationActor {Name} for Cluster {Id}",
				  ClusterRepresentation->GetName(), ClusterId);

		// Double check for sanity
		check(ClusterId == ClusterRepresentation->ClusterId);

		State->SetCorrespondingClusterActor(ClusterRepresentation);
		State->SetCorrespondingClusterId(ClusterId);

		if (State->GetPlayerType() == EPlayerType::nDisplayPrimary)
		{
			// We're the owner of the actor!
			ClusterRepresentation->SetOwner(NewPlayerController);
		}
	}

	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void ARWTHVRGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	if (const ARWTHVRPlayerState* State = Cast<ARWTHVRPlayerState>(NewPlayer->PlayerState); State != nullptr)
	{
		// Do we already have an auto-possessing pawn possessed?
		if (NewPlayer->GetPawn() && NewPlayer->GetPawn()->IsValidLowLevelFast())
		{
			UE_LOGFMT(Toolkit, Display,
					  "ARWTHVRGameModeBase::PostLogin: New player already auto-possessed a pawn, not spawning new one");
			Super::PostLogin(NewPlayer);
			return;
		}

		// If the new player is a secondary nDisplay node, spawn it only as a Spectator
		// Potentially we can use MustSpectate instead.
		UClass* PawnClass;
		if (State->GetPlayerType() == EPlayerType::nDisplaySecondary)
		{
			// For now, simply use the BP approach of spawning the pawn here. Can do this in a better way potentially.
			PawnClass = SpectatorClass;
		}
		else
			PawnClass = DefaultPawnClass;

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;
		const AActor* StartSpot = FindPlayerStart(NewPlayer);

		// If a start spot wasn't found,
		if (StartSpot == nullptr)
		{
			// Check for a previously assigned spot
			if (NewPlayer->StartSpot != nullptr)
			{
				StartSpot = NewPlayer->StartSpot.Get();
				UE_LOG(Toolkit, Warning, TEXT("RestartPlayer: Player start not found, using last start spot"));
			}
		}

		// Spawn and possess the pawn
		APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, StartSpot->GetTransform(), SpawnInfo);
		NewPlayer->Possess(ResultPawn);
	}

	Super::PostLogin(NewPlayer);
}
