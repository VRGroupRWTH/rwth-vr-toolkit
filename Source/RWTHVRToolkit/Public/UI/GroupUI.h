// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GroupUI.generated.h"


class APlayerGroup;
class AGroupInterfaceActor;
/**
 *
 */
UCLASS(Abstract)
class RWTHVRTOOLKIT_API UGroupUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnGroupsUpdated();

	UFUNCTION(BlueprintImplementableEvent)
	void OnGroupUpdated(int32 GroupId, APlayerGroup* Group);
	
	// Set by AGroupInterfaceActor itself
	UPROPERTY(BLueprintReadOnly)
	TObjectPtr<AGroupInterfaceActor> GroupInterfaceActor;
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupCreate();
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupOwnership(int32 GroupId = -1);
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupJoin(int32 GroupId, FName ColocatedGroupName = NAME_None);
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupLeave(int32 GroupId = -1);	
};
