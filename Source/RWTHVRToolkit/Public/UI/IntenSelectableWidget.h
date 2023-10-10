// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Interface.h"
#include "IntenSelectableWidget.generated.h"

UINTERFACE(BlueprintType)

class UIntenSelectableWidget : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IIntenSelectableWidget
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCoordinates();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector2D GetUISize();
};