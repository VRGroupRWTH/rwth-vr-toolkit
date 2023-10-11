// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableMultiPointScoring.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableMultiPointScoring::UIntenSelectableMultiPointScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FHitResult, float> UIntenSelectableMultiPointScoring::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = Super::GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	FHitResult Result = FHitResult{GetOwner(), nullptr, Point, FVector::ForwardVector};
	return TPair<FHitResult, float>{Result, Score};
}

FVector UIntenSelectableMultiPointScoring::GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const
{
	if(PointsToSelect.Num() == 0)
	{
		return this->GetComponentLocation();
	}

	FVector ClosestPoint = this->GetComponentTransform().TransformPositionNoScale(PointsToSelect[0]);
	float MinDistance = UKismetMathLibrary::GetPointDistanceToLine(ClosestPoint, Point, Direction);
	
	for(const FVector P : PointsToSelect)
	{
		const FVector PointToCheck = this->GetComponentTransform().TransformPositionNoScale(P);
		const float Dist = UKismetMathLibrary::GetPointDistanceToLine(PointToCheck, Point, Direction);
		if(Dist < MinDistance)
		{
			MinDistance = Dist;
			ClosestPoint = PointToCheck;
		}
	}
	return ClosestPoint;
}
