// Fill out your copyright notice in the Description page of Project Settings.


#include "Groups/PlayerGroupManager.h"

#include "Net/UnrealNetwork.h"
#include "Groups/PlayerGroup.h"
#include "Utility/RWTHVRUtilities.h"


// Sets default values
APlayerGroupManager::APlayerGroupManager()
{
	bReplicates = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot"));
}


// Called when the game starts or when spawned
void APlayerGroupManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerGroupManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerGroupManager, PlayerGroups);
}

// Server Only
APlayerGroup* APlayerGroupManager::CreateGroup(APawn* InitialMember)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Error, "CreateGroup can only be executed on authority.");
		return nullptr;
	}
	
	auto InitialTransform = InitialMember ? InitialMember->GetTransform() : FTransform::Identity;
	
	auto NewGroup = GetWorld()->SpawnActor<APlayerGroup>(APlayerGroup::StaticClass(), InitialTransform);
	int32 Index = PlayerGroups.Add(NewGroup);
	
	NewGroup->OnGroupUpdatedDelegate.AddUObject(this, &APlayerGroupManager::OnGroupUpdated);
	/*
	NewGroup->OnGroupUpdatedDelegate.AddLambda([Index, &Delegate = OnGroupUpdatedByIndexDelegate](APlayerGroup* Group)
	{
		Delegate.Broadcast(Index, Group);
	});
	*/
	UE_LOGFMT(Toolkit, Display, "IsBound: {b}", NewGroup->OnGroupUpdatedDelegate.IsBoundToObject(this));
	
	
	if (InitialMember)
	{
		NewGroup->JoinGroup(InitialMember);
	}
	
	// Broadcast on server - clients will broadcast on RepNotify
	OnGroupsUpdatedDelegate.Broadcast();
	
	return NewGroup;
}

// Server Only
APlayerGroup* APlayerGroupManager::JoinGroup(int32 GroupId, APawn* Pawn, FName ColocatedGroupName)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Error, "JoinGroup can only be executed on authority.");
		return nullptr;
	}
	
	// Check index again
	if (!PlayerGroups.IsValidIndex(GroupId))
	{
		UE_LOGFMT(Toolkit, Warning, "Requested to join group with index {i}, which does not exists.", GroupId);
		return nullptr;
	}
	
	auto Group = PlayerGroups[GroupId];
	if (Group->JoinGroup(Pawn, ColocatedGroupName))
		return Group;
	else return nullptr;
}

void APlayerGroupManager::LeaveGroup(int32 GroupId, APawn* Pawn)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Error, "LeaveGroup can only be executed on authority.");
		return;
	}
	
	// Check index again
	if (!PlayerGroups.IsValidIndex(GroupId))
	{
		UE_LOGFMT(Toolkit, Warning, "Requested to leave group with index {i}, which does not exists.", GroupId);
		return;
	}
	
	auto Group = PlayerGroups[GroupId];
	Group->LeaveGroup(Pawn);
}

void APlayerGroupManager::OnGroupUpdated(APlayerGroup* UpdatedGroup) const
{
	if (int32 Idx = PlayerGroups.IndexOfByKey(UpdatedGroup); Idx != -1)
		OnGroupUpdatedByIndexDelegate.Broadcast(Idx, UpdatedGroup);
}
