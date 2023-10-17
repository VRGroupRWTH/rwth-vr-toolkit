// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRPlayerState.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

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

void ARWTHVRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ARWTHVRPlayerState, PlayerType, SharedParams);
}

void ARWTHVRPlayerState::SetPlayerType(const EPlayerType NewPlayerType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ARWTHVRPlayerState, PlayerType, this);
	PlayerType = NewPlayerType;
}
