// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Pawn/ClusterRepresentationActor.h"
#include "RWTHVRGameModeBase.generated.h"

/**
 * Simple GameModeBase extension that checks for join options such that we can distinguish between primary
 * and secondary nodes in clusters. Could be moved to a different place as well, but quite reasonable here for now.
 * Keep in mind that GameMode only exists on the server!
 */
UCLASS()
class RWTHVRTOOLKIT_API ARWTHVRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	
	ARWTHVRGameModeBase(const FObjectInitializer& ObjectInitializer);

protected:
	/**
	 * Checks the connection options to see whether we're running in a cave multi user environment.
	 * If we are, set the player types correspondingly.
	 */
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
								  const FString& Options, const FString& Portal) override;

	/**
	 * Checks the player type of the NewPlayer. If it has been set to nDisplaySecondary, spawn a spectator pawn and
	 * possess. If not, spawn a DefaultPawnClass Pawn and Possess it (Should be BP_VirtualRealityPawn to make sense).
	 */
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

	UPROPERTY()
	TMap<int32, AClusterRepresentationActor*> ConnectedClusters;
};
