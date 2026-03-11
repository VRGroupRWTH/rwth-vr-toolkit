// Fill out your copyright notice in the Description page of Project Settings.


#include "Groups/PlayerGroup.h"

#include "Core/ClientTransformReplication.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/Navigation/TeleportationComponent.h"
#include "Utility/RWTHVRUtilities.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

// Sets default values
APlayerGroup::APlayerGroup()
{
	bReplicates = true;
	SetReplicateMovement(true);
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot"));
	
	ClientTransformReplication = CreateDefaultSubobject<UClientTransformReplication>("ClientTransformReplication");
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
	
	auto AttachmentTransformRules = FAttachmentTransformRules::KeepWorldTransform;
	
	if (ColocatedGroupName != NAME_None)
	{
		auto ColocatedGroup = ColocatedGroups.FindByPredicate([&](const FColocatedGroup& Group){return Group.ColocatedGroupName == ColocatedGroupName;});
		AttachmentTransformRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;

		if (ColocatedGroup)
		{
			// Group already exists
			ColocatedGroup->ColocatedPlayerPawns.Add(Pawn);
		}
		else
		{
			// Make new group and add player
			FColocatedGroup Grp;
			Grp.ColocatedGroupName = ColocatedGroupName;
			Grp.ColocatedPlayerPawns.Add(Pawn);
			Grp.Origin = FTransform::Identity; // Todo support multiple different origins
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
	
	if (Pawn->IsAttachedTo(this))
	{
		Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	bool bRequiresChangedBroadcast = false;
	if (GroupedPlayerPawns.RemoveSingle(Pawn) > 0)
		bRequiresChangedBroadcast = true;
	
	if (Pawn == OwningPawn)
	{
		// todo this might be dangerous?
		ChangeOwnership(nullptr);
		bRequiresChangedBroadcast = true;
	}
	
	if (bRequiresChangedBroadcast)
		OnGroupUpdatedDelegate.Broadcast(this);
}

void APlayerGroup::ChangeOwnership(APawn* NewPawn)
{
	if (!HasAuthority())
		return;
	
	HandleOldOwner(OwningPawn, NewPawn);
	
	SetOwner(NewPawn);
	OwningPawn = NewPawn;
	
	HandleNewOwner(NewPawn);
	
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

void APlayerGroup::OnRep_OwningPawnChanged(APawn* OldPawn)
{
	HandleOldOwner(OldPawn, OwningPawn);
	HandleNewOwner(OwningPawn);
	OnGroupUpdatedDelegate.Broadcast(this);
}

void APlayerGroup::HandleOldOwner(APawn* OldOwner, APawn* NewOwner)
{
	// If we're removing ownership from a pawn, let the previous owner pawn control its own movement again
	if (OldOwner && OldOwner != NewOwner)
	{
		// Currently only teleport component is relevant... Should do an interface or something like that here
		OldOwner->ForEachComponent<UMovementComponentBase>(false, [this](UMovementComponentBase* Component)
		{
			Component->ResetActorToMove();
		});	
	}
}

void APlayerGroup::HandleNewOwner(APawn* NewOwner)
{
	// Set the new owner to move the group
	if (NewOwner)
	{
		// Currently only teleport component is relevant... Should do an interface or something like that here
		NewOwner->ForEachComponent<UMovementComponentBase>(false, [this](UMovementComponentBase* Component)
		{
			Component->ChangeActorToMove(this);
		});		
	}
}
