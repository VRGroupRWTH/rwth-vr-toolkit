// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "DoorOverlayData.generated.h"

/**
 * Used as a parent-class in the overlay widget. Like this we can access the UMG properties in C++
 */
UCLASS()
class RWTHVRCLUSTER_API UDoorOverlayData : public UUserWidget
{
	GENERATED_BODY()

public:
	// These declarations are magically bound to the UMG blueprints elements,
	// if they are named the same
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CornerText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBorder* BlackBox;
};
