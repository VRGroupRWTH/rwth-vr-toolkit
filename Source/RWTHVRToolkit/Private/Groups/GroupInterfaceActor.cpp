// Fill out your copyright notice in the Description page of Project Settings.


#include "Groups/GroupInterfaceActor.h"

#include "Components/WidgetComponent.h"
#include "Groups/PlayerGroupManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/GroupUI.h"
#include "Utility/RWTHVRUtilities.h"


AGroupInterfaceActor::AGroupInterfaceActor()
{
	bReplicates = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot"));


	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarionetteWidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetDrawSize(FVector2D(1920, 1080));
	auto UIBPAssetPath = TEXT("Blueprint'/RWTHVRToolkit/UI/WBP_GroupUI'");
	ConstructorHelpers::FClassFinder<UGroupUI> UIBPAsset(UIBPAssetPath);
	if (!UIBPAsset.Succeeded())
	{
		UE_LOGFMT(Toolkit, Error,
				  "Could not find /RWTHVRToolkit/UI/WBP_GroupUI blueprint when constructing Group Interface Actor");
		return;
	}
	GroupUIBPClass = UIBPAsset.Class;
}

void AGroupInterfaceActor::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}

void AGroupInterfaceActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGroupInterfaceActor, CurrentGroup);
	DOREPLIFETIME(AGroupInterfaceActor, PlayerGroupManager);
}

// Called when the game starts or when spawned
void AGroupInterfaceActor::Initialize()
{
	if (bInitializedOnClient)
	{
		return;
	}
	
	if (!HasLocalNetOwner())
	{
		UE_LOGFMT(Toolkit, Display, "GroupInterfaceActor has no Local Net Owner");
		return;
	}
	
	if (!PlayerGroupManager)
	{
		UE_LOGFMT(Toolkit, Display, "GroupInterfaceActor has no PlayerGroupManager");
		return;
	}
	
	auto LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	
	if (!GroupUI && GroupUIBPClass)
	{
		GroupUI = CreateWidget<UGroupUI, APlayerController*>(LocalPlayerController, GroupUIBPClass);
		GroupUI->AddToViewport();
		GroupUI->SetDesiredSizeInViewport({600, 500});
		GroupUI->GroupInterfaceActor = this;
	}

	if (GroupUI)
	{
		PlayerGroupManager->OnGroupsUpdatedDelegate.AddUFunction(GroupUI, "OnGroupsUpdated");
		PlayerGroupManager->OnGroupUpdatedByIndexDelegate.AddUFunction(GroupUI, "OnGroupUpdated");
	}
	bInitializedOnClient = true;

}
void AGroupInterfaceActor::OnRep_PlayerGroupManagerSet()
{
	Initialize();
}

void AGroupInterfaceActor::RequestCreateGroup(APawn* InitialMember)
{
	if (HasAuthority())
		CreateGroupInternal(InitialMember);
	else
		ServerCreateGroupRpc(InitialMember);
}

void AGroupInterfaceActor::RequestJoinGroup(int32 GroupId, FName ColocatedGroupName)
{
	// As Groups are replicated, check if the index even exists:
	if (!GetPlayerGroupManager()->PlayerGroups.IsValidIndex(GroupId))
	{
		UE_LOGFMT(Toolkit, Warning, "Requested to join group with index {i}, which does not exists.", GroupId);
		return;
	}

	UE_LOGFMT(Toolkit, Display, "Requested to join group with index {i}, colocated group name: {G}", GroupId,
			  ColocatedGroupName);

	if (HasAuthority())
		JoinGroupInternal(GroupId, ColocatedGroupName);
	else
		ServerJoinGroupRpc(GroupId, ColocatedGroupName);
}

int32 AGroupInterfaceActor::GetCurrentGroupIndex(int32 GroupId)
{
	if (GroupId < 0)
	{
		if (CurrentGroup)
		{
			if (int32 Index = GetPlayerGroupManager()->PlayerGroups.IndexOfByKey(CurrentGroup); Index != INDEX_NONE)
			{
				return Index;
			}
		}
	}
	return GroupId;
}

void AGroupInterfaceActor::RequestLeaveGroup(int32 GroupId)
{
	int32 Index = GetCurrentGroupIndex(GroupId);
	UE_LOGFMT(Toolkit, Display, "Requested to leave group with index {i}", Index);
	
	if (!GetPlayerGroupManager()->PlayerGroups.IsValidIndex(GroupId))
	{
		UE_LOGFMT(Toolkit, Error, "Invalid group index {i}", Index);
		return;
	}

	if (HasAuthority())
		LeaveGroupInternal(Index);
	else
		ServerLeaveGroupRpc(Index);
}

void AGroupInterfaceActor::RequestGroupOwnership(int32 GroupId)
{
	int32 Index = GetCurrentGroupIndex(GroupId);
	UE_LOGFMT(Toolkit, Display, "Requested ownership of group with index {i}", Index);
	
	if (!GetPlayerGroupManager()->PlayerGroups.IsValidIndex(GroupId))
	{
		UE_LOGFMT(Toolkit, Error, "Invalid group index {i}", Index);
		return;
	}
	
	if (HasAuthority())
		GetGroupOwnershipInternal(GroupId);
	else
		ServerGetGroupOwnershipRpc(GroupId);	
}

void AGroupInterfaceActor::ServerLeaveGroupRpc_Implementation(int32 GroupId)
{
	LeaveGroupInternal(GroupId);
}

// Server Only
void AGroupInterfaceActor::LeaveGroupInternal(int32 GroupId)
{
	if (auto Pawn = GetOwningPawn())
	{
		GetPlayerGroupManager()->LeaveGroup(GroupId, Pawn);
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "Could not determine owning pawn of group interface actor, which is required for leaving a group.");
	}
}

// Server Only
void AGroupInterfaceActor::GetGroupOwnershipInternal(int32 GroupId)
{
	if (auto Pawn = GetOwningPawn())
	{
		// also checked before even sending rpc, but double check on server here.
		if (!PlayerGroupManager->PlayerGroups.IsValidIndex(GroupId))
		{
			UE_LOGFMT(Toolkit, Error, "Invalid group index {i}", GroupId);
			return;
		}
		
		auto Group = PlayerGroupManager->PlayerGroups[GroupId];
		Group->ChangeOwnership(Pawn);
	}
}


APlayerGroupManager* AGroupInterfaceActor::GetPlayerGroupManager()
{
	return PlayerGroupManager;
	/*
	// Should have been set by gamemode already, but replication might not have happened yet
	if (!PlayerGroupManager)
	{
		auto PGM = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerGroupManager::StaticClass());
		if (!PGM)
		{
			UE_LOGFMT(Toolkit, Warning, "Could not find PlayerGroupManager when setting up Group Interface Actor");
			return nullptr;
		}
		PlayerGroupManager = Cast<APlayerGroupManager>(PGM);
	}
	return  PlayerGroupManager;
	*/
}

void AGroupInterfaceActor::ServerJoinGroupRpc_Implementation(int32 GroupId, FName ColocatedGroupName)
{
	JoinGroupInternal(GroupId, ColocatedGroupName);
}

void AGroupInterfaceActor::ServerGetGroupOwnershipRpc_Implementation(int32 GroupId)
{
	GetGroupOwnershipInternal(GroupId);	
}

// Only on server
void AGroupInterfaceActor::CreateGroupInternal(APawn* InitialMember)
{
	if (!GetPlayerGroupManager())
	{
		UE_LOGFMT(Toolkit, Error, "PlayerGroupManager not set when creating group, aborting.");
		return;
	}

	PlayerGroupManager->CreateGroup(InitialMember);
}


// Server only
void AGroupInterfaceActor::JoinGroupInternal(int32 GroupId, FName ColocatedGroupName)
{
	if (auto Pawn = GetOwningPawn())
	{
		CurrentGroup = GetPlayerGroupManager()->JoinGroup(GroupId, Pawn, ColocatedGroupName);
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "Could not determine owning pawn of group interface actor, which is required for joining a group.");
	}
}

void AGroupInterfaceActor::ServerCreateGroupRpc_Implementation(APawn* InitialMember)
{
	CreateGroupInternal(InitialMember);
}

APawn* AGroupInterfaceActor::GetOwningPawn() const
{
	// Find the pawn we belong to:

	auto PotentialPawn = Cast<APawn>(GetOwner());
	if (!PotentialPawn)
	{
		if (const auto PotentialPC = Cast<APlayerController>(GetOwner()))
		{
			PotentialPawn = PotentialPC->GetPawn();
		}
	}
	return PotentialPawn;
}