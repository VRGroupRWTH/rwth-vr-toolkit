// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/GroupUI.h"

#include "Groups/GroupInterfaceActor.h"
#include "Groups/PlayerGroupManager.h"
#include "Kismet/GameplayStatics.h"

void UGroupUI::InitUI(AGroupInterfaceActor* InterfaceActor)
{
	GroupInterfaceActor = InterfaceActor;
	if (!GroupInterfaceActor)
	{
		// this might still be null, as the interface actor might not exist yet.
		// in this case, the interface actor will additionally search for all UI actors and initialize them.
		GroupInterfaceActor = Cast<AGroupInterfaceActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AGroupInterfaceActor::StaticClass()));
	}
	
	if (GroupInterfaceActor)
	{
		UpdatedHandle = GroupInterfaceActor->PlayerGroupManager->OnGroupsUpdatedDelegate.AddUFunction(this, "OnGroupsUpdated");
		UpdatedByIndexHandle = GroupInterfaceActor->PlayerGroupManager->OnGroupUpdatedByIndexDelegate.AddUFunction(this, "OnGroupUpdated");
	}
}

void UGroupUI::NativeConstruct()
{
	Super::NativeConstruct();
	InitUI();
}

void UGroupUI::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (GroupInterfaceActor && GroupInterfaceActor->PlayerGroupManager)
	{
		GroupInterfaceActor->PlayerGroupManager->OnGroupsUpdatedDelegate.Remove(UpdatedHandle);
		GroupInterfaceActor->PlayerGroupManager->OnGroupUpdatedByIndexDelegate.Remove(UpdatedByIndexHandle);
	}
}

void UGroupUI::RequestGroupCreate()
{
	if (GroupInterfaceActor)
		GroupInterfaceActor->RequestCreateGroup();
}

void UGroupUI::RequestGroupOwnership(int32 GroupId)
{
	if (GroupInterfaceActor)
		GroupInterfaceActor->RequestGroupOwnership(GroupId);
}

void UGroupUI::RequestGroupJoin(int32 GroupId, FName ColocatedGroupName)
{
	if (GroupInterfaceActor)
		GroupInterfaceActor->RequestJoinGroup(GroupId, ColocatedGroupName);
}

void UGroupUI::RequestGroupLeave(int32 GroupId)
{
	if (GroupInterfaceActor)
		GroupInterfaceActor->RequestLeaveGroup(GroupId);
}