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
	
	void InitUI(AGroupInterfaceActor* InterfaceActor = nullptr);
	
protected:
	// Set by AGroupInterfaceActor itself
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AGroupInterfaceActor> GroupInterfaceActor;
	
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupCreate();
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupOwnership(int32 GroupId = -1);
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupJoin(int32 GroupId, FName ColocatedGroupName = NAME_None);
	
	UFUNCTION(BlueprintCallable)
	void RequestGroupLeave(int32 GroupId = -1);	
	
private:	
	FDelegateHandle UpdatedHandle;
	FDelegateHandle UpdatedByIndexHandle;
	

};
