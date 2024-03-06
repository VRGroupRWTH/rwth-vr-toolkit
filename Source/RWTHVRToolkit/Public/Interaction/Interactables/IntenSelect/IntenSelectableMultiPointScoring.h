// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableMultiPointScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectableMultiPointScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableMultiPointScoring();

	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		TArray<FVector> PointsToSelect{FVector::UpVector * 50, FVector::DownVector * 50, FVector::RightVector * 50, FVector::LeftVector * 50};;

};
