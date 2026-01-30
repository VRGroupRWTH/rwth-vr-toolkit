// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerGroup.h"
#include "GameFramework/Actor.h"
#include "PlayerGroupManager.generated.h"

class APlayerGroup;

DECLARE_MULTICAST_DELEGATE(FOnGroupsUpdated);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGroupUpdatedByIndex, int32, APlayerGroup*);

UCLASS()
class RWTHVRTOOLKIT_API APlayerGroupManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerGroupManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(ReplicatedUsing=OnRep_GroupsUpdated, BlueprintReadOnly)
	TArray<APlayerGroup*> PlayerGroups;
	
	FOnGroupsUpdated OnGroupsUpdatedDelegate;
	FOnGroupUpdatedByIndex OnGroupUpdatedByIndexDelegate;

	void virtual GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	APlayerGroup* CreateGroup(APawn* InitialMember);
	
	UFUNCTION()
	APlayerGroup* JoinGroup(int32 GroupId, APawn* Pawn, FName ColocatedGroupName = NAME_None);
	
	UFUNCTION()
	void LeaveGroup(int32 GroupId, APawn* Pawn);
	
	UFUNCTION()
	void OnRep_GroupsUpdated() const 
	{
		// Update delegate bindings on clients as well
		for (APlayerGroup* PlayerGroup : PlayerGroups)
		{
			if (!PlayerGroup->OnGroupUpdatedDelegate.IsBoundToObject(this))
			{
				PlayerGroup->OnGroupUpdatedDelegate.AddUObject(this, &APlayerGroupManager::OnGroupUpdated);
			}
		}
		
		OnGroupsUpdatedDelegate.Broadcast();
	}
	
	UFUNCTION()
	void OnGroupUpdated(APlayerGroup* UpdatedGroup) const;
	
};
