// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerGroup.generated.h"


USTRUCT(BlueprintType)
struct RWTHVRTOOLKIT_API FColocatedGroup
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName ColocatedGroupName;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<APawn>> ColocatedPlayerPawns;

	FColocatedGroup()
	{
	}
	
	/**
	* @param Ar FArchive to read or write from.
	* @param Map PackageMap used to resolve references to UObject*
	* @param bOutSuccess return value to signify if the serialization was succesfull (if false, an error will be logged by the calling function)
	*
	* @return return true if the serialization was fully mapped. If false, the property will be considered 'dirty' and will replicate again on the next update.
	* This is needed for UActor* properties. If an actor's Actorchannel is not fully mapped, properties referencing it must stay dirty.
	* Note that UPackageMap::SerializeObject returns false if an object is unmapped. Generally, you will want to return false from your ::NetSerialize
	* if you make any calls to ::SerializeObject that return false.
	*
	*/
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << ColocatedGroupName;
		Ar << ColocatedPlayerPawns;
		return true;
	}
};

class APlayerGroup;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGroupUpdated, APlayerGroup*);

// TODO: Maybe make the group an actual pawn which can be possessed and acts as a kind of "vehicle" 
UCLASS()
class RWTHVRTOOLKIT_API APlayerGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerGroup();
	
	UFUNCTION()
	bool JoinGroup(APawn* Pawn, FName ColocatedGroupName = NAME_None);	

	UFUNCTION()
	void LeaveGroup(APawn* Pawn);	
	
	FOnGroupUpdated OnGroupUpdatedDelegate;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:		
		
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
		
	// TSets aren't replicated, so we have to do this the annoying way
	UPROPERTY(ReplicatedUsing=OnRep_GroupedPlayerPawns, BlueprintReadOnly)
	TArray<TObjectPtr<APawn>> GroupedPlayerPawns;

	UPROPERTY(ReplicatedUsing=OnRep_ColocatedGroups, BlueprintReadOnly)
	TArray<FColocatedGroup> ColocatedGroups;
	
private:
	UFUNCTION()
	void OnRep_GroupedPlayerPawns();
	
	UFUNCTION()
	void OnRep_ColocatedGroups();
};
