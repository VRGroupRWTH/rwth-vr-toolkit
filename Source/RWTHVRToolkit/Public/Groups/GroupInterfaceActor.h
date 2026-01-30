// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroupInterfaceActor.generated.h"


class APlayerGroup;
class UWidgetComponent;
class APlayerGroupManager;
class UGroupUI;

// This should be on a pawn / playercontroller...
// In theory, is only relevant to respective local controlled client (like a playercontroller)
// On the other hand, we could use it to get info about other players' groups
UCLASS()
class RWTHVRTOOLKIT_API AGroupInterfaceActor : public AActor
{
	friend class APlayerGroupManager;
	
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGroupInterfaceActor();
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void RequestCreateGroup(APawn* InitialMember = nullptr);
	
	void RequestJoinGroup(int32 GroupId, FName ColocatedGroupName = NAME_None);
	int32 GetCurrentGroupIndex(int32 GroupId);

	void RequestLeaveGroup(int32 GroupId);
	
	void RequestGroupOwnership(int32 GroupId);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_PlayerGroupManagerSet)
	TObjectPtr<APlayerGroupManager> PlayerGroupManager;
	
protected:
	void Initialize();	
	
private:
	
	UFUNCTION()
	void OnRep_PlayerGroupManagerSet();
	
	UFUNCTION(Server, Reliable)
	void ServerCreateGroupRpc(APawn* InitialMember);
	
	UFUNCTION(Server, Reliable)
	void ServerJoinGroupRpc(int32 GroupId, FName ColocatedGroupName);

	UFUNCTION(Server, Reliable)
	void ServerLeaveGroupRpc(int32 GroupId);
	
	UFUNCTION(Server, Reliable)
	void ServerGetGroupOwnershipRpc(int32 GroupId);
	
	UFUNCTION()
	void CreateGroupInternal(APawn* InitialMember);
	
	APawn* GetOwningPawn() const;

	UFUNCTION()
	void JoinGroupInternal(int32 GroupId, FName ColocatedGroupName);
	
	UFUNCTION()
	void LeaveGroupInternal(int32 GroupId);
	
	UFUNCTION()
	void GetGroupOwnershipInternal(int32 GroupId);
	
	APlayerGroupManager* GetPlayerGroupManager();
	
	UPROPERTY()
	TObjectPtr<UGroupUI> GroupUI;
	
	TSubclassOf<UGroupUI> GroupUIBPClass;
	
	UPROPERTY()
	TObjectPtr<UWidgetComponent> WidgetComponent;
	
	UPROPERTY(Replicated)
	TObjectPtr<APlayerGroup> CurrentGroup;

	bool bInitializedOnClient = false;
};
