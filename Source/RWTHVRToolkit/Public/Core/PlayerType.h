// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PlayerType.generated.h"

/**
 * Enum that defines the various player types. Potentially this could be changed into the Enum/FName hybrid that's often
 * used, which would make this runtime extendable. Probably unnecessary.
 */
UENUM(BlueprintType)
enum class EPlayerType : uint8
{
	nDisplayPrimary UMETA(DisplayName = "nDisplay Primary"),
	nDisplaySecondary UMETA(DisplayName = "nDisplay Secondary"),
	Desktop UMETA(DisplayName = "Desktop"),
	HMD UMETA(DisplayName = "HMD")
};
