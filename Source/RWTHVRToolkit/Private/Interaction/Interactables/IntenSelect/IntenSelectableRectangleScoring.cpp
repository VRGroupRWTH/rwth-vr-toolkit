// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/Interactables/IntenSelect/IntenSelectableRectangleScoring.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableRectangleScoring::UIntenSelectableRectangleScoring() { PrimaryComponentTick.bCanEverTick = true; }

TPair<FHitResult, float>
UIntenSelectableRectangleScoring::GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
														const float ConeBackwardShiftDistance, const float ConeAngle,
														const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = Super::GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point,
								  LastValue, DeltaTime);
	FHitResult Result = FHitResult{GetOwner(), nullptr, Point, FVector::ForwardVector};
	return TPair<FHitResult, float>{Result, Score};
}

bool UIntenSelectableRectangleScoring::LineToLineIntersection(const FVector& FromA, const FVector& FromB,
															  const FVector& ToA, const FVector& ToB,
															  FVector& OutIntersection)
{
	const FVector Da = ToA - FromA;
	const FVector DB = ToB - FromB;
	const FVector DC = FromB - FromA;

	const FVector CrossDaDb = FVector::CrossProduct(Da, DB);
	const float Prod = CrossDaDb.X * CrossDaDb.X + CrossDaDb.Y * CrossDaDb.Y + CrossDaDb.Z * CrossDaDb.Z;

	const float Res = FVector::DotProduct(FVector::CrossProduct(DC, DB), FVector::CrossProduct(Da, DB) / Prod);
	if (Res >= -0.02f && Res <= 1.02f)
	{
		OutIntersection = FromA + Da * FVector(Res, Res, Res);
		return true;
	}

	return false;
}

FVector UIntenSelectableRectangleScoring::GetClosestSelectionPointTo(const FVector& Point,
																	 const FVector& Direction) const
{
	const FVector X = this->GetRightVector() * XLength;
	const FVector Y = this->GetUpVector() * YLength;

	const FVector CornerWorld00 =
		this->GetComponentTransform().TransformPosition(FVector::ZeroVector) - (X / 2) - (Y / 2);
	const FVector CornerWorld10 = CornerWorld00 + X;
	const FVector CornerWorld01 = CornerWorld00 + Y;
	const FVector CornerWorld11 = CornerWorld00 + X + Y;

	const FVector PlaneNormal =
		FVector::CrossProduct(CornerWorld10 - CornerWorld00, CornerWorld01 - CornerWorld00).GetSafeNormal();

	FVector Intersection;
	float T;
	UKismetMathLibrary::LinePlaneIntersection_OriginNormal(Point, Point + Direction * 10000, CornerWorld00, PlaneNormal,
														   T, Intersection);

	FVector LocalIntersection = this->GetComponentTransform().InverseTransformPosition(Intersection);

	if (LocalIntersection.Y > XLength / 2)
	{
		LocalIntersection.Y = XLength / 2;
	}
	else if (LocalIntersection.Y < -XLength / 2)
	{
		LocalIntersection.Y = -XLength / 2;
	}

	if (LocalIntersection.Z > YLength / 2)
	{
		LocalIntersection.Z = YLength / 2;
	}
	else if (LocalIntersection.Z < -YLength / 2)
	{
		LocalIntersection.Z = -YLength / 2;
	}

	if (OnlyOutline)
	{
		const float DistToBottom = LocalIntersection.Z + (YLength / 2);
		const float DistToLeft = LocalIntersection.Y + (XLength / 2);

		if (LocalIntersection.Z < 0)
		{
			if (LocalIntersection.Y < 0)
			{
				// Bottom and left
				if (DistToLeft < DistToBottom)
				{
					// snap left
					LocalIntersection.Y = -(XLength / 2);
				}
				else
				{
					// snap bottom
					LocalIntersection.Z = -(YLength / 2);
				}
			}
			else
			{
				// bottom and right
				if (XLength - DistToLeft < DistToBottom)
				{
					// snap right
					LocalIntersection.Y = XLength / 2;
				}
				else
				{
					// snap bottom
					LocalIntersection.Z = -(YLength / 2);
				}
			}
		}
		else
		{
			if (LocalIntersection.Y < 0)
			{
				// top and left
				if (DistToLeft < YLength - DistToBottom)
				{
					// snap left
					LocalIntersection.Y = -(XLength / 2);
				}
				else
				{
					// snap top
					LocalIntersection.Z = (YLength / 2);
				}
			}
			else
			{
				// top and right
				if (XLength - DistToLeft < YLength - DistToBottom)
				{
					// snap right
					LocalIntersection.Y = XLength / 2;
				}
				else
				{
					// snap top
					LocalIntersection.Z = (YLength / 2);
				}
			}
		}
	}

	return this->GetComponentTransform().TransformPosition(LocalIntersection);
}

void UIntenSelectableRectangleScoring::TickComponent(float DeltaTime, ELevelTick TickType,
													 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DrawDebug)
	{
		const FVector X = this->GetRightVector() * XLength;
		const FVector Y = this->GetUpVector() * YLength;

		const FVector CornerWorld00 =
			this->GetComponentTransform().TransformPosition(FVector::ZeroVector) - (X / 2) - (Y / 2);
		const FVector CornerWorld10 = CornerWorld00 + X;
		const FVector CornerWorld01 = CornerWorld00 + Y;
		const FVector CornerWorld11 = CornerWorld00 + X + Y;

		DrawDebugLine(GetWorld(), CornerWorld00, CornerWorld01, FColor::Green, false, -1, 0, 2);
		DrawDebugLine(GetWorld(), CornerWorld00, CornerWorld10, FColor::Green, false, -1, 0, 2);
		DrawDebugLine(GetWorld(), CornerWorld01, CornerWorld11, FColor::Green, false, -1, 0, 2);
		DrawDebugLine(GetWorld(), CornerWorld10, CornerWorld11, FColor::Green, false, -1, 0, 2);
	}
}
