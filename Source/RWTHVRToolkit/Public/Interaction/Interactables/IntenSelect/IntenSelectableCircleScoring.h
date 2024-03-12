// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCircleScoring.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableCircleScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableCircleScoring();

	UPROPERTY(EditAnywhere)
	bool OnlyOutline = true;

	UPROPERTY(EditAnywhere)
	float Radius = 50;

	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;
};
