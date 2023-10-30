// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Boilerplate, copies properties to new state
void ARWTHVRPlayerState::CopyProperties(class APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (IsValid(PlayerState))
	{
		ARWTHVRPlayerState* RWTHVRPlayerState = Cast<ARWTHVRPlayerState>(PlayerState);
		if (IsValid(RWTHVRPlayerState))
		{
			RWTHVRPlayerState->SetPlayerType(GetPlayerType());
		}
	}
}

// Boilerplate
void ARWTHVRPlayerState::OverrideWith(class APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	if (IsValid(PlayerState))
	{
		const ARWTHVRPlayerState* RWTHVRPlayerState = Cast<ARWTHVRPlayerState>(PlayerState);
		if (IsValid(RWTHVRPlayerState))
		{
			SetPlayerType(RWTHVRPlayerState->GetPlayerType());
		}
	}
}

// Replicate our property similar to the other state properties
void ARWTHVRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ARWTHVRPlayerState, PlayerType, SharedParams);
}

void ARWTHVRPlayerState::ServerSetPlayerTypeRpc_Implementation(const EPlayerType NewPlayerType)
{
	SetPlayerType(NewPlayerType);
}

void ARWTHVRPlayerState::SetPlayerType(const EPlayerType NewPlayerType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ARWTHVRPlayerState, PlayerType, this);
	PlayerType = NewPlayerType;
}

void ARWTHVRPlayerState::RequestSetPlayerType(const EPlayerType NewPlayerType)
{
	if (HasAuthority())
	{
		SetPlayerType(NewPlayerType);
	}
	else
	{
		ServerSetPlayerTypeRpc(NewPlayerType);
	}
}
