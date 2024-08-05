// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RWTHVRPlayerState.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Utility/RWTHVRUtilities.h"

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
			RWTHVRPlayerState->SetCorrespondingClusterId(CorrespondingClusterId);
			RWTHVRPlayerState->SetCorrespondingClusterActor(CorrespondingClusterActor);
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
			SetCorrespondingClusterId(RWTHVRPlayerState->CorrespondingClusterId);
			SetCorrespondingClusterActor(RWTHVRPlayerState->CorrespondingClusterActor);
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
	DOREPLIFETIME_WITH_PARAMS_FAST(ARWTHVRPlayerState, CorrespondingClusterId, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ARWTHVRPlayerState, CorrespondingClusterActor, SharedParams);
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
void ARWTHVRPlayerState::SetCorrespondingClusterId(int32 NewCorrespondingClusterId)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Warning, "ARWTHVRPlayerState: Cannot set cluster Id on non-authority!");
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ARWTHVRPlayerState, CorrespondingClusterId, this);
	CorrespondingClusterId = NewCorrespondingClusterId;
}
void ARWTHVRPlayerState::SetCorrespondingClusterActor(AClusterRepresentationActor* NewCorrespondingClusterActor)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Warning, "ARWTHVRPlayerState: Cannot set cluster actor ref on non-authority!");
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ARWTHVRPlayerState, CorrespondingClusterActor, this);
	CorrespondingClusterActor = NewCorrespondingClusterActor;
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
