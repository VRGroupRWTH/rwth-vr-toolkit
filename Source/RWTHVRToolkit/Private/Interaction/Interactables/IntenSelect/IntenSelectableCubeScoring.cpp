#include "Interaction/Interactables/IntenSelect/IntenSelectableCubeScoring.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableCubeScoring::UIntenSelectableCubeScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetRelativeScale3D(FVector::One() * 100);
}

TPair<FHitResult, float>
UIntenSelectableCubeScoring::GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
												   const float ConeBackwardShiftDistance, const float ConeAngle,
												   const float LastValue, const float DeltaTime)
{
	const FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = Super::GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point,
								  LastValue, DeltaTime);
	FHitResult Result = FHitResult{GetOwner(), nullptr, Point, FVector::ForwardVector};
	return TPair<FHitResult, float>{Result, Score};
}

FVector UIntenSelectableCubeScoring::GetClosestPointToRectangle(const FVector& StartPoint, const FVector& Direction,
																const FVector& Corner00, const FVector& Corner01,
																const FVector& Corner10) const
{
	const float LengthX = FVector::Distance(Corner00, Corner10);
	const float LengthY = FVector::Distance(Corner00, Corner01);
	const FVector PlaneNormal = FVector::CrossProduct(Corner10 - Corner00, Corner01 - Corner00).GetSafeNormal();

	FVector Intersection;
	float T;
	UKismetMathLibrary::LinePlaneIntersection_OriginNormal(StartPoint, StartPoint + Direction * 10000, Corner00,
														   PlaneNormal, T, Intersection);

	FVector LocalIntersection = this->GetComponentTransform().InverseTransformPosition(Intersection);

	LocalIntersection.Y = FMath::Clamp(LocalIntersection.Y, -LengthX / 2, LengthX / 2);
	LocalIntersection.Z = FMath::Clamp(LocalIntersection.Z, -LengthY / 2, LengthY / 2);


	return this->GetComponentTransform().TransformPosition(LocalIntersection);
}

bool UIntenSelectableCubeScoring::LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
														 const FVector& ToB, FVector& OutIntersection)
{
	const FVector DistanceA = ToA - FromA;
	const FVector DistanceB = ToB - FromB;
	const FVector DistanceC = FromB - FromA;

	const FVector CrossProductAB = FVector::CrossProduct(DistanceA, DistanceB);
	const float Prod =
		CrossProductAB.X * CrossProductAB.X + CrossProductAB.Y * CrossProductAB.Y + CrossProductAB.Z * CrossProductAB.Z;

	const float Result = FVector::DotProduct(FVector::CrossProduct(DistanceC, DistanceB),
											 FVector::CrossProduct(DistanceA, DistanceB) / Prod);

	constexpr float IntersectionThreshold = 0.02f;
	if (Result >= -IntersectionThreshold && Result <= 1 + IntersectionThreshold)
	{
		OutIntersection = FromA + DistanceA * FVector(Result, Result, Result);
		return true;
	}

	return false;
}

FVector UIntenSelectableCubeScoring::GetClosestSelectionPointTo(const FVector& RayOrigin,
																const FVector& RayDirection) const
{
	auto Scale = GetRelativeTransform().GetScale3D();
	const FVector ForwardVectorScaled = this->GetForwardVector() * Scale.X;
	const FVector RightVectorScaled = this->GetRightVector() * Scale.Y;
	const FVector UpVectorScaled = this->GetUpVector() * Scale.Z;

	TArray<FPlane> CubeSides;

	// bottom
	const FVector BottomWorld = this->GetComponentTransform().TransformPositionNoScale(-UpVectorScaled / 2);
	CubeSides.Add(FPlane{BottomWorld, -this->GetUpVector()});

	// top
	const FVector TopWorld = this->GetComponentTransform().TransformPositionNoScale(UpVectorScaled / 2);
	CubeSides.Add(FPlane{TopWorld, this->GetUpVector()});

	// left
	const FVector LeftWorld = this->GetComponentTransform().TransformPositionNoScale(-RightVectorScaled / 2);
	CubeSides.Add(FPlane{LeftWorld, -this->GetRightVector()});

	// right
	const FVector RightWorld = this->GetComponentTransform().TransformPositionNoScale(RightVectorScaled / 2);
	CubeSides.Add(FPlane{RightWorld, this->GetRightVector()});

	// front
	const FVector FrontWorld = this->GetComponentTransform().TransformPositionNoScale(-ForwardVectorScaled / 2);
	CubeSides.Add(FPlane{FrontWorld, -this->GetForwardVector()});

	// back
	const FVector BackWorld = this->GetComponentTransform().TransformPositionNoScale(ForwardVectorScaled / 2);
	CubeSides.Add(FPlane{BackWorld, this->GetForwardVector()});

	float MinDistance = TNumericLimits<float>::Max();
	FVector ClosestPoint = GetComponentLocation();
	bool IsSet = false;
	for (FPlane Plane : CubeSides)
	{
		const FVector PlaneToRayOrigin = RayOrigin - Plane.GetOrigin();
		if (FVector::DotProduct(PlaneToRayOrigin.GetSafeNormal(), Plane.GetNormal()) < 0 && BackFaceCulling)
		{
			if (DrawDebug)
			{
				DrawDebugSolidPlane(GetWorld(), Plane, GetComponentLocation(), 20, FColor::Red.WithAlpha(9), false, -1,
									0);
			}
			continue;
		}
		else
		{
			if (DrawDebug)
			{
				DrawDebugSolidPlane(GetWorld(), Plane, GetComponentLocation(), 20, FColor::Green.WithAlpha(9), false,
									-1, 0);
			}
		}


		FVector CurrentPoint = FMath::RayPlaneIntersection(RayOrigin, RayDirection, Plane);
		FVector CurrentPointLocal = GetComponentTransform().InverseTransformPositionNoScale(CurrentPoint);

		Scale = GetRelativeTransform().GetScale3D();
		CurrentPointLocal.X = FMath::Clamp(CurrentPointLocal.X, -Scale.X / 2, Scale.X / 2);
		CurrentPointLocal.Y = FMath::Clamp(CurrentPointLocal.Y, -Scale.Y / 2, Scale.Y / 2);
		CurrentPointLocal.Z = FMath::Clamp(CurrentPointLocal.Z, -Scale.Z / 2, Scale.Z / 2);

		if (OnlyOutline)
		{
			const float XSnapDist = (Scale.X / 2) - FMath::Abs(CurrentPointLocal.X);
			const float YSnapDist = (Scale.Y / 2) - FMath::Abs(CurrentPointLocal.Y);
			const float ZSnapDist = (Scale.Z / 2) - FMath::Abs(CurrentPointLocal.Z);

			bool SnapX = true;
			bool SnapY = true;
			bool SnapZ = true;

			if (FVector::Parallel(Plane.GetNormal(), GetRightVector()))
			{
				if (XSnapDist < ZSnapDist)
				{
					SnapZ = false;
				}
				else
				{
					SnapX = false;
				}
			}
			else if (FVector::Parallel(Plane.GetNormal(), GetUpVector()))
			{
				if (XSnapDist < YSnapDist)
				{
					SnapY = false;
				}
				else
				{
					SnapX = false;
				}
			}
			else if (FVector::Parallel(Plane.GetNormal(), GetForwardVector()))
			{
				if (YSnapDist < ZSnapDist)
				{
					SnapZ = false;
				}
				else
				{
					SnapY = false;
				}
			}


			if (SnapX)
			{
				if (CurrentPointLocal.X > 0)
				{
					CurrentPointLocal.X = Scale.X / 2;
				}
				else
				{
					CurrentPointLocal.X = -Scale.X / 2;
				}
			}
			if (SnapY)
			{
				if (CurrentPointLocal.Y > 0)
				{
					CurrentPointLocal.Y = Scale.Y / 2;
				}
				else
				{
					CurrentPointLocal.Y = -Scale.Y / 2;
				}
			}
			if (SnapZ)
			{
				if (CurrentPointLocal.Z > 0)
				{
					CurrentPointLocal.Z = Scale.Z / 2;
				}
				else
				{
					CurrentPointLocal.Z = -Scale.Z / 2;
				}
			}
		}

		CurrentPoint = GetComponentTransform().TransformPositionNoScale(CurrentPointLocal);

		const float Distance = FMath::PointDistToLine(CurrentPoint, RayDirection, RayOrigin);

		if (Distance < 0.001)
		{
			if (MinDistance < 0.001)
			{
				const float DistToPlayerOld =
					IsSet ? FVector::Distance(RayOrigin, ClosestPoint) : TNumericLimits<float>::Max();
				const float DistToPlayerNew = FVector::Distance(RayOrigin, CurrentPoint);

				if (DistToPlayerNew < DistToPlayerOld)
				{
					MinDistance = Distance;
					ClosestPoint = CurrentPoint;
					IsSet = true;
				}
			}
			else
			{
				MinDistance = Distance;
				ClosestPoint = CurrentPoint;
				IsSet = true;
			}
		}
		else
		{
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestPoint = CurrentPoint;
				IsSet = true;
			}
		}
	}

	if (DrawDebug)
		DrawDebugBox(GetWorld(), GetComponentLocation(), FVector(Scale.X, Scale.Y, Scale.Z) / 2,
					 GetComponentRotation().Quaternion(), FColor::Green, false, -1, 0, 2);
	return ClosestPoint;
}

void UIntenSelectableCubeScoring::TickComponent(float DeltaTime, ELevelTick TickType,
												FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
