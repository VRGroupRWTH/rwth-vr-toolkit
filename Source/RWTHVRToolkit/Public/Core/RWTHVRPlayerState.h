// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerType.h"
#include "GameFramework/PlayerState.h"
#include "RWTHVRPlayerState.generated.h"

enum class EPlayerType : uint8;
/**
 * Extension of the PlayerState that additionally holds information about what type the player is.
 * E.g. nDisplayPrimary, nDisplaySecondary, Regular (ideally split into HMD | Desktop etc)
 */
UCLASS()
class RWTHVRTOOLKIT_API ARWTHVRPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	/** Replicated player type for this player*/
	UPROPERTY(Replicated, Category=PlayerState, BlueprintGetter=GetPlayerType, meta=(AllowPrivateAccess))
	EPlayerType PlayerType = EPlayerType::Desktop;

public:

	UFUNCTION(BlueprintGetter)
	EPlayerType GetPlayerType() const
	{
		return PlayerType;
	}
	
	void SetPlayerType(EPlayerType NewType);

	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
