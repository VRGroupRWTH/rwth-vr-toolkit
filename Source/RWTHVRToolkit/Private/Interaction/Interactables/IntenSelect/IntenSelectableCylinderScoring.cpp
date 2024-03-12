// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interactables/IntenSelect/IntenSelectableCylinderScoring.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableCylinderScoring::UIntenSelectableCylinderScoring() { PrimaryComponentTick.bCanEverTick = true; }

TPair<FHitResult, float>
UIntenSelectableCylinderScoring::GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
													   const float ConeBackwardShiftDistance, const float ConeAngle,
													   const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score =
		GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	FHitResult Result = FHitResult{GetOwner(), nullptr, Point, FVector::ForwardVector};
	return TPair<FHitResult, float>{Result, Score};
}

bool UIntenSelectableCylinderScoring::LineToLineIntersection(const FVector& FromA, const FVector& FromB,
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

FVector UIntenSelectableCylinderScoring::GetClosestSelectionPointTo(const FVector& Point,
																	const FVector& Direction) const
{
	const FVector CylinderStartWorld = this->GetComponentTransform().TransformPosition(LinePoints[0]);
	const FVector CylinderEndWorld = this->GetComponentTransform().TransformPosition(LinePoints[1]);
	const FVector CylinderDir = CylinderEndWorld - CylinderStartWorld;

	const FVector CrossProd = UKismetMathLibrary::Cross_VectorVector(CylinderDir, Direction); // v
	const FVector LineDifference = CylinderStartWorld - Point; // u

	// Project v onto u =>
	const FVector Proj = LineDifference.ProjectOnTo(CrossProd);
	const float ProjLength = Proj.Size();

	const FVector OffsetPoint = Point + Proj;

	const FVector FromA = OffsetPoint;
	const FVector FromB = CylinderStartWorld;
	const FVector ToA = OffsetPoint + Direction * 10000;
	const FVector ToB = CylinderEndWorld;
	FVector Result;
	LineToLineIntersection(FromA, FromB, ToA, ToB, Result);

	const FVector LineDirRes = Result - CylinderStartWorld;

	if (LineDirRes.Size() > CylinderDir.Size())
	{
		Result = CylinderEndWorld;
	}

	if (!LineDirRes.GetSafeNormal().Equals(CylinderDir.GetSafeNormal()))
	{
		Result = CylinderStartWorld;
	}

	const FVector ToSphere = Result - Point;
	const FVector Projection = ToSphere.ProjectOnTo(Direction);
	const FVector ProjectionToSphere = ToSphere - Projection;

	if (ProjLength >= Radius)
	{
		FVector ShiftResult = ProjectionToSphere.GetSafeNormal() * Radius;
		ShiftResult -= ShiftResult.ProjectOnTo(CylinderDir);
		return Result - ShiftResult;
	}
	else
	{
		FVector ShiftResult = ProjectionToSphere.GetSafeNormal() * ProjLength;
		ShiftResult -= ShiftResult.ProjectOnTo(CylinderDir);
		return Result - ShiftResult;
	}
}

void UIntenSelectableCylinderScoring::TickComponent(float DeltaTime, ELevelTick TickType,
													FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DrawDebug)
	{
		const FVector StartWorld = this->GetComponentTransform().TransformPosition(LinePoints[0]);
		const FVector EndWorld = this->GetComponentTransform().TransformPosition(LinePoints[1]);

		DrawDebugCylinder(GetWorld(), StartWorld, EndWorld, Radius, 20, FColor::Green, false, 0, 0, 2);
	}
}
