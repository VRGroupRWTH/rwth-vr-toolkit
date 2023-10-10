// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableSphereScoring.h"

#include "DrawDebugHelpers.h"
#include "Algo/IndexOf.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableSphereScoring::UIntenSelectableSphereScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectableSphereScoring::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	return TPair<FVector, float>{Point, Score};
}

FVector UIntenSelectableSphereScoring::GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const
{
	const FVector CenterWorld = this->GetComponentLocation();

	const FVector ToSphere = CenterWorld - Point;

	const FVector Projection = ToSphere.ProjectOnTo(Direction);
	const FVector ProjectionToSphere = ToSphere - Projection;

	FVector Result = CenterWorld - ProjectionToSphere.GetSafeNormal() * Radius;

	if(!OnlyOutline)
	{
		const float t = FVector::DotProduct(ToSphere, Direction.GetSafeNormal());
		const FVector TPoint = Point + Direction.GetSafeNormal() * t;
		const float Y = (CenterWorld - TPoint).Size();
		
		if(Y <= Radius)
		{
			const float X = + FMath::Sqrt((Radius * Radius) - (Y * Y));

			const FVector Result1 = Point + Direction.GetSafeNormal() * (t - X);
			const FVector Result2 = Point + Direction.GetSafeNormal() * (t + X);

			if(FVector::Distance(Point, Result1) < FVector::Distance(Point, Result2))
			{
				Result = Result1;
			}else
			{
				Result = Result2;
			}
		}
		
		/*
		TArray<FHitResult> Out;
		const float Dist = FVector::Distance(Point, GetComponentLocation());
		if(GetWorld()->LineTraceMultiByChannel(Out, Point, Point + (Direction.GetSafeNormal() * Dist), ECollisionChannel::ECC_Visibility))
		{
			for(auto Hit : Out)
			{
				if(Hit.GetActor() == GetOwner())
				{
					Result = Hit.ImpactPoint;
					break;
				}
			}
		}*/
	}

	if(DrawDebug)
	{
		DrawDebugSphere(GetWorld(), CenterWorld, Radius, 20, FColor::Green, false, -1, 0, 1);
	}
	
	return Result;
}
