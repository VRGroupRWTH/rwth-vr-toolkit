// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Interface.h"
#include "Targetable.generated.h"

UINTERFACE(BlueprintType)
class RWTHVRTOOLKIT_API UTargetable: public UInterface
{
	// has to be empty, this is Unreals syntax to make it visible in blueprints
	GENERATED_UINTERFACE_BODY()
};

class ITargetable
{
	GENERATED_IINTERFACE_BODY()

public:
	// function that will be called when targetable actor is targeted, the world pos of the target is returned
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnTargeted(FVector WorldPositionOfTarget);
	
	//function that will be called when a targetable actor gets focused
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnTargetedEnter();

	//function that will be called when a targetable actor loses focused
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnTargetedLeave();
};
