// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

protected:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	                              const FString& Options, const FString& Portal) override;
};
