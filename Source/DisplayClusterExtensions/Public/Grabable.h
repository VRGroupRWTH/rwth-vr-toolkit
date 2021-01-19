// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Interface.h"
#include "Grabable.generated.h"

UINTERFACE(BlueprintType)
class DISPLAYCLUSTEREXTENSIONS_API UGrabable : public UInterface
{
	// has to be empty, this is Unreals syntax to make it visible in blueprints
	GENERATED_UINTERFACE_BODY()
};

class IGrabable
{
	GENERATED_IINTERFACE_BODY()

public:
	// function that will be called when grabbed by a pawn
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnGrabbed();
	
	// called when pawn released the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnReleased();

};
