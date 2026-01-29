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
void AGroupInterfaceActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGroupInterfaceActor, CurrentGroup);
}

// Called when the game starts or when spawned
void AGroupInterfaceActor::BeginPlay()
{
	Super::BeginPlay();

	// Should have been set by gamemode already
	if (!PlayerGroupManager)
	{
		auto PGM = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerGroupManager::StaticClass());
		if (!PGM)
		{
			UE_LOGFMT(Toolkit, Warning, "Could not find PlayerGroupManager when setting up Group Interface Actor");
			return;
		}

		PlayerGroupManager = Cast<APlayerGroupManager>(PGM);
	}

	if (!GroupUI && GroupUIBPClass)
	{
		auto LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
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
	if (!PlayerGroupManager->PlayerGroups.IsValidIndex(GroupId))
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

void AGroupInterfaceActor::RequestLeaveGroup(int32 GroupId)
{
	if (GroupId < 0)
	{
		// Leave current group
		if (CurrentGroup)
		{
			if (int32 Index = PlayerGroupManager->PlayerGroups.IndexOfByKey(CurrentGroup); Index != INDEX_NONE)
			{
				GroupId = Index;
			}
		}
	}
	UE_LOGFMT(Toolkit, Display, "Requested to leave group with index {i}", GroupId);

	if (HasAuthority())
		LeaveGroupInternal(GroupId);
	else
		ServerLeaveGroupRpc(GroupId);
}

void AGroupInterfaceActor::ServerLeaveGroupRpc_Implementation(int32 GroupId) { LeaveGroupInternal(GroupId); }

void AGroupInterfaceActor::LeaveGroupInternal(int32 GroupId)
{
	if (auto Pawn = GetOwningPawn())
	{
		PlayerGroupManager->LeaveGroup(GroupId, Pawn);
	}
	else
	{
		UE_LOGFMT(Toolkit, Error,
				  "Could not determine owning pawn of group interface actor, which is required for leaving a group.");
	}
}

void AGroupInterfaceActor::ServerJoinGroupRpc_Implementation(int32 GroupId, FName ColocatedGroupName)
{
	JoinGroupInternal(GroupId, ColocatedGroupName);
}

// Only on server
void AGroupInterfaceActor::CreateGroupInternal(APawn* InitialMember)
{
	if (!PlayerGroupManager)
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
		CurrentGroup = PlayerGroupManager->JoinGroup(GroupId, Pawn, ColocatedGroupName);
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
	if (!PlayerGroupManager)
	{
		UE_LOGFMT(Toolkit, Error, "PlayerGroupManager not set when trying to get pawn, aborting.");
		return nullptr;
	}

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