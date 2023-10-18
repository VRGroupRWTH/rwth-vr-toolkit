// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRGameModeBase.h"

#include "Core/RWTHVRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


FString ARWTHVRGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
                                           const FString& Options, const FString& Portal)
{
	const FString NodeNameKey = "node";
	const FString PrimaryNodeIdKey = "PrimaryNodeId";

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
			State->SetPlayerType(Type);		
		}
	}
	
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);	
}
