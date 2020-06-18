// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "UObject/Interface.h"
#include "Clickable.generated.h"


UINTERFACE(BlueprintType)
class DISPLAYCLUSTEREXTENSIONS_API UClickable : public UInterface
{
	// has to be empty, this is Unreals syntax to make it visible in blueprints
	GENERATED_UINTERFACE_BODY()
};

class IClickable
{
	GENERATED_IINTERFACE_BODY()

public:
	// function that will be called when clickable actor got clicked
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnClicked();
	

};

