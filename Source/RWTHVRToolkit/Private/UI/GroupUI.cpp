// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/GroupUI.h"

#include "Groups/GroupInterfaceActor.h"

void UGroupUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGroupUI::RequestGroupCreate()
{
	GroupInterfaceActor->RequestCreateGroup();
}

void UGroupUI::RequestGroupOwnership()
{
	
}

void UGroupUI::RequestGroupJoin(int32 GroupId, FName ColocatedGroupName)
{
	GroupInterfaceActor->RequestJoinGroup(GroupId, ColocatedGroupName);
}

void UGroupUI::RequestGroupLeave(int32 GroupId)
{
	GroupInterfaceActor->RequestLeaveGroup(GroupId);
}