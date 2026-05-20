#include "Core/RWTHVRGameModeBase.h"

#include "Core/RWTHVRPlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Groups/GroupInterfaceActor.h"
#include "Groups/PlayerGroupManager.h"
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
			const EPlayerType Type =
				URWTHVRUtilities::IsPrimaryNode() ? EPlayerType::nDisplayPrimary : EPlayerType::nDisplaySecondary;
			State->RequestSetPlayerType(Type);
		}
		State->SetCorrespondingClusterId(ClusterId);
	}

	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void ARWTHVRGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	if (ARWTHVRPlayerState* State = Cast<ARWTHVRPlayerState>(NewPlayer->PlayerState); State != nullptr)
	{
		// If we're in none-standalone netmode, this is only executed on the server, as the GM only exists there.
		// On standalone, this is executed on every node.
		int32 ClusterId = State->GetCorrespondingClusterId();
		if (ClusterId >= 0) // we're either standalone (0) or in an acutal cluster
		{
			AClusterRepresentationActor** ClusterRepresentationPtr = ConnectedClusters.Find(ClusterId);
			AClusterRepresentationActor* ClusterRepresentation;
			if (!ClusterRepresentationPtr)
			{
				// No actor there yet, spawn it
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Name = FName(*FString::Printf(TEXT("ClusterRepresentation_%d"), ClusterId));
				SpawnParameters.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
				ClusterRepresentation = GetWorld()->SpawnActor<AClusterRepresentationActor>(SpawnParameters);
				UE_LOGFMT(Toolkit, Display,
						  "ARWTHVRGameModeBase: Spawned ClusterRepresentationActor {Name} for Cluster {Id}",
						  ClusterRepresentation->GetName(), ClusterId);
				ConnectedClusters.Add(ClusterId, ClusterRepresentation);
			}
			else
			{
				ClusterRepresentation = *ClusterRepresentationPtr;
			}

			UE_LOGFMT(Toolkit, Display, "ARWTHVRGameModeBase: Using ClusterRepresentationActor {Name} for Cluster {Id}",
					  *ClusterRepresentation->GetName(), ClusterId);

			// Double check for sanity
			check(ClusterRepresentation != nullptr);

			State->SetCorrespondingClusterActor(ClusterRepresentation);

			if (State->GetPlayerType() == EPlayerType::nDisplayPrimary)
			{
				// We're the owner of the actor!
				ClusterRepresentation->SetOwner(NewPlayer);
			}
		}

		// Do we already have an auto-possessing pawn possessed?
		if (NewPlayer->GetPawn() && NewPlayer->GetPawn()->IsValidLowLevelFast())
		{
			UE_LOGFMT(Toolkit, Display,
					  "ARWTHVRGameModeBase::PostLogin: New player already auto-possessed a pawn, not spawning new one");
			Super::PostLogin(NewPlayer);
			return;
		}

		// When we're not in standalone:
		// If the new player is a secondary nDisplay node, spawn it only as a Spectator
		// Potentially we can use MustSpectate instead.
		UClass* PawnClass;
		if (GetNetMode() != NM_Standalone && State->GetPlayerType() == EPlayerType::nDisplaySecondary)
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

		if (GetNetMode() == NM_Standalone)
		{
			const FName BaseName = PawnClass->HasAnyFlags(RF_ClassDefaultObject)
				? PawnClass->GetFName()
				: *PawnClass->GetFName().GetPlainNameString();

			SpawnInfo.Name = BaseName;
			SpawnInfo.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		}

		// Spawn and possess the pawn
		APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, StartSpot->GetTransform(), SpawnInfo);
		NewPlayer->Possess(ResultPawn);
	}

	Super::PostLogin(NewPlayer);

	if (bEnableGroups)
	{
		AActor* NewOwner = NewPlayer->GetPawn();

		if (!NewOwner)
		{
			UE_LOGFMT(Toolkit, Warning,
					  "PostLogin: Failed to get player pawn for player {NP} when trying to spawn GroupInterfaceActor "
					  "and setting its owner. Setting owner to player controller instead",
					  NewPlayer->GetName());
			NewOwner = NewPlayer;
		}

		auto GroupInterfaceActor = GetWorld()->SpawnActorDeferred<AGroupInterfaceActor>(
			AGroupInterfaceActor::StaticClass(), FTransform::Identity, NewOwner, Cast<APawn>(NewOwner),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// attachment not required but should look cleaner
		GroupInterfaceActor->AttachToActor(NewOwner, FAttachmentTransformRules::SnapToTargetIncludingScale);
		
		if (!PlayerGroupManager)
		{
			UE_LOGFMT(Toolkit, Error,
					  "PostLogin: Failed to get Player Group Manager. This should have been initialized in InitGame, "
					  "something went wrong and code might need to be rewritten.");
		}
		else
		{
			GroupInterfaceActor->PlayerGroupManager = PlayerGroupManager;
		}
		GroupInterfaceActor->FinishSpawning(FTransform::Identity);

		UE_LOGFMT(Toolkit, Display, "Finished spawning group interface actor {GI} for new player {NP}",
				  GroupInterfaceActor->GetName(), NewPlayer->GetName());
	}
}
void ARWTHVRGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}
void ARWTHVRGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	if (bEnableGroups)
	{
		PlayerGroupManager = GetWorld()->SpawnActor<APlayerGroupManager>();
	}
	
	Super::InitGame(MapName, Options, ErrorMessage);	
}
