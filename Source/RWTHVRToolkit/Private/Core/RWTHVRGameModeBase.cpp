// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRGameModeBase.h"

#include "Core/RWTHVRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


FString ARWTHVRGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
                                           const FString& Options, const FString& Portal)
{
	// Used by the DisplayClusterNetDriver/Connection to handshake nodes. Could use their types directly
	// but I don't really want to introduce a hard dependency here.
	const FString NodeNameKey = "node";
	const FString PrimaryNodeIdKey = "PrimaryNodeId";

	// Check if we're using our custom PlayerState so that we can save the player type there.
	// If not, just ingore all related args.
	ARWTHVRPlayerState* State = Cast<ARWTHVRPlayerState>(NewPlayerController->PlayerState);
		
	if (State != nullptr)
	{		
		if (UGameplayStatics::HasOption(Options, PrimaryNodeIdKey))
		{
			const FString PrimaryNodeId = UGameplayStatics::ParseOption(Options, PrimaryNodeIdKey);

			// When the primary node is a listen server, it apparently doesn't get the node option...
			// Could additionally check for listen, but this should be save enough.
			const FString NodeName = UGameplayStatics::HasOption(Options, NodeNameKey)
				                         ? UGameplayStatics::ParseOption(Options, NodeNameKey)
				                         : PrimaryNodeId;
			
			const EPlayerType Type = NodeName == PrimaryNodeId ? EPlayerType::nDisplayPrimary : EPlayerType::nDisplaySecondary;			
			State->RequestSetPlayerType(Type);		
		}
	}
	
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);	
}
