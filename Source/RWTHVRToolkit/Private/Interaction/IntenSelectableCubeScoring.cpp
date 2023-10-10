// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableCubeScoring.h"
#include "DrawDebugHelpers.h"
#include "Intersection/IntrRay3AxisAlignedBox3.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableCubeScoring::UIntenSelectableCubeScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectableCubeScoring::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = Super::GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	return TPair<FVector, float>{Point, Score};
}

FVector UIntenSelectableCubeScoring::GetClosestPointToRectangle(const FVector& StartPoint, const FVector& Direction, const FVector& Corner00, const FVector& Corner01, const FVector& Corner10, const FVector& Corner11) const
{
	const float X = FVector::Distance(Corner00, Corner10);
	const float Y = FVector::Distance(Corner00, Corner01);
	const FVector PlaneNormal = FVector::CrossProduct(Corner10 - Corner00, Corner01 - Corner00).GetSafeNormal();

	FVector Intersection;
	float T;
	UKismetMathLibrary::LinePlaneIntersection_OriginNormal(StartPoint, StartPoint + Direction * 10000, Corner00, PlaneNormal, T, Intersection);

	FVector LocalIntersection = this->GetComponentTransform().InverseTransformPosition(Intersection);

	if(LocalIntersection.Y > X / 2)
	{
		LocalIntersection.Y = X / 2;
	}else if(LocalIntersection.Y < -X / 2)
	{
		LocalIntersection.Y = -X / 2;
	}
	
	if(LocalIntersection.Z > Y / 2)
	{
		LocalIntersection.Z = Y / 2;
	}else if(LocalIntersection.Z < -Y / 2)
	{
		LocalIntersection.Z = -Y / 2;
	}

	/*
	if(OnlyOutline)
	{
		const float DistToBottom = LocalIntersection.Z + (YLength / 2);
		const float DistToLeft = LocalIntersection.Y + (XLength / 2);
		
		if(LocalIntersection.Z < 0)
		{
			if(LocalIntersection.Y < 0)
			{
				//Bottom and left
				if(DistToLeft < DistToBottom)
				{
					//snap left
					LocalIntersection.Y = -(XLength / 2);
				}else
				{
					//snap bottom
					LocalIntersection.Z = -(YLength / 2);
				}
			}else
			{
				//bottom and right
				if(XLength - DistToLeft < DistToBottom)
				{
					//snap right
					LocalIntersection.Y = XLength / 2;
				}else
				{
					//snap bottom
					LocalIntersection.Z = -(YLength / 2);
				}
			}
		}else
		{
			if(LocalIntersection.Y < 0)
			{
				//top and left
				if(DistToLeft < YLength - DistToBottom)
				{
					//snap left
					LocalIntersection.Y = -(XLength / 2);
				}else
				{
					//snap top
					LocalIntersection.Z = (YLength / 2);
				}
			}else
			{
				//top and right
				if(XLength - DistToLeft < YLength - DistToBottom)
				{
					//snap right
					LocalIntersection.Y = XLength / 2;
				}else
				{
					//snap top
					LocalIntersection.Z = (YLength / 2);
				}
			}
		}
	}
*/
	
	return this->GetComponentTransform().TransformPosition(LocalIntersection);
}

bool UIntenSelectableCubeScoring::LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA, const FVector& ToB, FVector& OutIntersection)
{
	const FVector Da = ToA - FromA;
	const FVector DB = ToB - FromB;
	const FVector DC = FromB - FromA;

	const FVector CrossDaDb = FVector::CrossProduct(Da, DB);
	const float Prod = CrossDaDb.X * CrossDaDb.X + CrossDaDb.Y * CrossDaDb.Y + CrossDaDb.Z * CrossDaDb.Z;

	const float Res = FVector::DotProduct(FVector::CrossProduct(DC, DB), FVector::CrossProduct(Da, DB) / Prod);
	if (Res >= -0.02f && Res <= 1.02f) {
		OutIntersection = FromA + Da * FVector(Res, Res, Res);
		return true;
	}
	
	return false;
}

FVector UIntenSelectableCubeScoring::GetClosestSelectionPointTo(const FVector& RayOrigin, const FVector& RayDirection)
{
	const FVector X = this->GetForwardVector() * XLength;
	const FVector Y = this->GetRightVector() * YLength;
	const FVector Z = this->GetUpVector() * ZLength;

	TArray<FPlane> CubeSides;
	
	//bottom
	const FVector BottomWorld = this->GetComponentTransform().TransformPositionNoScale(- Z / 2);
	CubeSides.Add(FPlane{BottomWorld, -this->GetUpVector()});

	//top
	const FVector TopWorld = this->GetComponentTransform().TransformPositionNoScale(Z / 2);
	CubeSides.Add(FPlane{TopWorld, this->GetUpVector()});
	
	//left
	const FVector LeftWorld = this->GetComponentTransform().TransformPositionNoScale(- Y / 2);
	CubeSides.Add(FPlane{LeftWorld, -this->GetRightVector()});

	//right
	const FVector RightWorld = this->GetComponentTransform().TransformPositionNoScale(Y / 2);
	CubeSides.Add(FPlane{RightWorld, this->GetRightVector()});

	//front
	const FVector FrontWorld = this->GetComponentTransform().TransformPositionNoScale(-X / 2);
	CubeSides.Add(FPlane{FrontWorld, -this->GetForwardVector()});

	//back
	const FVector BackWorld = this->GetComponentTransform().TransformPositionNoScale(X / 2);
	CubeSides.Add(FPlane{BackWorld, this->GetForwardVector()});

	/*
	const TRay3<float> Ray{Point, Direction, false};
	const TAxisAlignedBox3<float> Box;
	float OutT;
	if(TIntrRay3AxisAlignedBox3<float>::FindIntersection(Ray, Box, OutT))
	{
		
	}*/
	
	float MinDistance = TNumericLimits<float>::Max();
	FVector ClosestPoint = GetComponentLocation();
	bool IsSet = false;
	for(FPlane Plane : CubeSides)
	{
		const FVector PlaneToRayOrigin = RayOrigin - Plane.GetOrigin();
		if(FVector::DotProduct(PlaneToRayOrigin.GetSafeNormal(), Plane.GetNormal()) < 0 && BackFaceCulling)
		{
			if(DrawDebug)
			{
				DrawDebugSolidPlane(GetWorld(), Plane, GetComponentLocation(), 20, FColor::Red.WithAlpha(9), false, -1, 0);
			}
			continue;
		}else
		{
			if(DrawDebug)
			{
				DrawDebugSolidPlane(GetWorld(), Plane, GetComponentLocation(), 20, FColor::Green.WithAlpha(9), false, -1, 0);
			}
		}
		
		
		
		FVector CurrentPoint = FMath::RayPlaneIntersection(RayOrigin, RayDirection, Plane);
		FVector CurrentPointLocal = GetComponentTransform().InverseTransformPosition(CurrentPoint);
		
		CurrentPointLocal.X = FMath::Clamp(CurrentPointLocal.X, -XLength / 2, XLength / 2);
		CurrentPointLocal.Y = FMath::Clamp(CurrentPointLocal.Y, -YLength / 2, YLength / 2);
		CurrentPointLocal.Z = FMath::Clamp(CurrentPointLocal.Z, -ZLength / 2, ZLength / 2);

		if(OnlyOutline)
		{
			const float XSnapDist = (XLength/2) - FMath::Abs(CurrentPointLocal.X);
			const float YSnapDist = (YLength/2) - FMath::Abs(CurrentPointLocal.Y);
			const float ZSnapDist = (ZLength/2) - FMath::Abs(CurrentPointLocal.Z);

			bool SnapX = true;
			bool SnapY = true;
			bool SnapZ = true;
			
			if(FVector::Parallel(Plane.GetNormal(), GetRightVector()))
			{
				if(XSnapDist < ZSnapDist)
				{
					SnapZ = false;
				}else
				{
					SnapX = false;
				}
				
			}else if(FVector::Parallel(Plane.GetNormal(), GetUpVector()))
			{
				if(XSnapDist < YSnapDist)
				{
					SnapY = false;
				}else
				{
					SnapX = false;
				}
			}else if(FVector::Parallel(Plane.GetNormal(), GetForwardVector()))
			{
				if(YSnapDist < ZSnapDist)
				{
					SnapZ = false;
				}else
				{
					SnapY = false;
				}
			}
			
			if(SnapX)
			{
				if(CurrentPointLocal.X > 0)
				{
					CurrentPointLocal.X = XLength / 2;
				}else
				{
					CurrentPointLocal.X = -XLength / 2;
				}
			}
			if(SnapY)
			{
				if(CurrentPointLocal.Y > 0)
				{
					CurrentPointLocal.Y = YLength / 2;
				}else
				{
					CurrentPointLocal.Y = -YLength / 2;
				}
			}
			if(SnapZ)
			{
				if(CurrentPointLocal.Z > 0)
				{
					CurrentPointLocal.Z = ZLength / 2;
				}else
				{
					CurrentPointLocal.Z = -ZLength / 2;
				}
			}
		}

		CurrentPoint = GetComponentTransform().TransformPosition(CurrentPointLocal);

		const float Distance = FMath::PointDistToLine(CurrentPoint, RayDirection, RayOrigin);

		//DrawDebugPoint(GetWorld(), CurrentPoint, 10, FColor::Black.WithAlpha(1), false, -1, 0);
		//GEngine->AddOnScreenDebugMessage(INDEX_NONE, -1, FColor::Red, FString::SanitizeFloat(Distance));
		
		if(Distance < 0.001)
		{
			if(MinDistance < 0.001)
			{
				const float DistToPlayerOld = IsSet ? FVector::Distance(RayOrigin, ClosestPoint) : TNumericLimits<float>::Max();
				const float DistToPlayerNew = FVector::Distance(RayOrigin, CurrentPoint);
			
				if(DistToPlayerNew < DistToPlayerOld)
				{
					MinDistance = Distance;
					ClosestPoint = CurrentPoint;
					IsSet = true;
				}
			}else
			{
				MinDistance = Distance;
				ClosestPoint = CurrentPoint;
				IsSet = true;
			}			
		}else
		{
			if(Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestPoint = CurrentPoint;
				IsSet = true;
			}
		}		
	}

	if(DrawDebug) DrawDebugBox(GetWorld(), GetComponentLocation(), FVector(XLength, YLength, ZLength) / 2, GetComponentRotation().Quaternion(), FColor::Green, false, -1, 0, 2);
	return ClosestPoint;
}

void UIntenSelectableCubeScoring::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
