// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerType.h"
#include "GameFramework/PlayerState.h"
#include "RWTHVRPlayerState.generated.h"


class AClusterRepresentationActor;
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
	UPROPERTY(Replicated, Category = PlayerState, BlueprintGetter = GetPlayerType, meta = (AllowPrivateAccess))
	EPlayerType PlayerType = EPlayerType::Desktop;

	/** Replicated cluster ID for this player. Is -1 in case player is not a cluster*/
	UPROPERTY(Replicated, Category = PlayerState, BlueprintGetter = GetCorrespondingClusterId, meta = (AllowPrivateAccess))
	int32 CorrespondingClusterId = -1;

	/** Replicated cluster actor for this player. Is nullptr in case player is not a cluster*/
	UPROPERTY(Replicated, Category = PlayerState, BlueprintGetter = GetCorrespondingClusterActor,
			  meta = (AllowPrivateAccess))
	AClusterRepresentationActor* CorrespondingClusterActor = nullptr;
	
	UFUNCTION(Reliable, Server)
	void ServerSetPlayerTypeRpc(EPlayerType NewPlayerType);

	void SetPlayerType(EPlayerType NewPlayerType);

public:
	UFUNCTION(BlueprintGetter)
	EPlayerType GetPlayerType() const { return PlayerType; }

	UFUNCTION(BlueprintGetter)
	int32 GetCorrespondingClusterId() const { return CorrespondingClusterId; }

	UFUNCTION(BlueprintGetter)
	AClusterRepresentationActor* GetCorrespondingClusterActor() const { return CorrespondingClusterActor; }
	
	void SetCorrespondingClusterId(int32 NewCorrespondingClusterId);
	void SetCorrespondingClusterActor(AClusterRepresentationActor* NewCorrespondingClusterActor);

	UFUNCTION(BlueprintCallable)
	void RequestSetPlayerType(EPlayerType NewPlayerType);

	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
