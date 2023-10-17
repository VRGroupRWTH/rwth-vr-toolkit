// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RWTHVRGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class RWTHVRTOOLKIT_API ARWTHVRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal) override;
};
