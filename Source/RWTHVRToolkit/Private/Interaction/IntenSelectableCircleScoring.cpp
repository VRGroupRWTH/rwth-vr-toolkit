// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableCircleScoring.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableCircleScoring::UIntenSelectableCircleScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectableCircleScoring::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	return TPair<FVector, float>{Point, Score};
}

FVector UIntenSelectableCircleScoring::GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const
{
	const FVector CenterWorld = this->GetComponentLocation();
	const FVector CircleNormalWorld = this->GetComponentTransform().TransformPositionNoScale(FVector::ForwardVector) - CenterWorld;
	
	float t;
	FVector Intersect;
	if(!UKismetMathLibrary::LinePlaneIntersection_OriginNormal(Point, Point + Direction * 100000, CenterWorld, CircleNormalWorld, t, Intersect))
	{
		return CenterWorld;
	}

	const FVector CenterToPoint = Intersect - CenterWorld;

	FVector Result;
	if(OnlyOutline)
	{
		Result = (CenterToPoint.GetSafeNormal() * Radius) + CenterWorld;
	}else
	{
		const float DistanceToCenter = CenterToPoint.Size();
		
		if(DistanceToCenter >= Radius)
		{
			Result = (CenterToPoint.GetSafeNormal() * Radius) + CenterWorld;
		}else
		{
			Result = Intersect;
		}
	}

	FVector Y = CenterToPoint.GetSafeNormal();
	FVector Z = FVector::CrossProduct(Y, CircleNormalWorld.GetSafeNormal());

	//Y = FVector(0, 0, 1);
	//Z = FVector(1, 0, 0);
	DrawDebugCircle(GetWorld(), CenterWorld, Radius, 80, FColor::Green, false, -1, 0, 1, Y, Z, false);
	
	return Result;
}
