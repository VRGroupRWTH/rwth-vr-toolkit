// Fill out your copyright notice in the Description page of Project Settings.


#include "Groups/PlayerGroup.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Utility/RWTHVRUtilities.h"

// Sets default values
APlayerGroup::APlayerGroup()
{
	bReplicates = true;
	SetReplicateMovement(true);
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot"));
}

// Called when the game starts or when spawned
void APlayerGroup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Currently assumes all co-located clients share the same virtual coordinate system. This might not be the case if e.g. a Standalone Quest joins.
// Can only be called on the server
bool APlayerGroup::JoinGroup(APawn* Pawn, FName ColocatedGroupName)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(Toolkit, Warning, "Joining a group requires authority, should only be called on the server. Aborting");
	}
	
	if (GroupedPlayerPawns.Contains(Pawn))
	{
		UE_LOGFMT(Toolkit, Warning, "Groups: {P} is already in a group.", Pawn->GetName());
		return false;
	}
	GroupedPlayerPawns.Add(Pawn);
	
	auto AttachmentTransformRules = FAttachmentTransformRules::KeepRelativeTransform;
	
	if (ColocatedGroupName != NAME_None)
	{
		auto ColocatedGroup = ColocatedGroups.FindByPredicate([&](const FColocatedGroup& Group){return Group.ColocatedGroupName == ColocatedGroupName;});
		if (ColocatedGroup)
		{
			// Group already exists
			ColocatedGroup->ColocatedPlayerPawns.Add(Pawn);
			AttachmentTransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
		}
		else
		{
			// Make new group and add player
			FColocatedGroup Grp;
			Grp.ColocatedGroupName = ColocatedGroupName;
			Grp.ColocatedPlayerPawns.Add(Pawn);
			ColocatedGroups.Add(Grp);
		}
	}
	Pawn->AttachToActor(this, AttachmentTransformRules);
	
	OnGroupUpdatedDelegate.Broadcast(this);
	return true;
}

void APlayerGroup::LeaveGroup(APawn* Pawn)
{
	// Remove from colocated
	for (auto& Group : ColocatedGroups)
	{
		Group.ColocatedPlayerPawns.RemoveSingle(Pawn);
	}
	
	// Remove from full list
	if (GroupedPlayerPawns.RemoveSingle(Pawn) > 0)
		OnGroupUpdatedDelegate.Broadcast(this);
}

void APlayerGroup::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerGroup, GroupedPlayerPawns);
	DOREPLIFETIME(APlayerGroup, ColocatedGroups);
}

void APlayerGroup::OnRep_GroupedPlayerPawns()
{
	OnGroupUpdatedDelegate.Broadcast(this);
}

void APlayerGroup::OnRep_ColocatedGroups()
{
	OnGroupUpdatedDelegate.Broadcast(this);
}
