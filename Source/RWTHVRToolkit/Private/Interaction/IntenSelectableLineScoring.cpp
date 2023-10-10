// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableLineScoring.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIntenSelectableLineScoring::UIntenSelectableLineScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectableLineScoring::GetBestPointScorePair(const FVector& ConeOrigin,
                                                                         const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
                                                                         const float LastValue, const float DeltaTime)
{
	FVector Point = GetClosestSelectionPointTo(ConeOrigin, ConeForwardDirection);
	float Score = GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	return TPair<FVector, float>{Point, Score};
}

bool UIntenSelectableLineScoring::LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA, const FVector& ToB, FVector& OutIntersection)
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

FVector UIntenSelectableLineScoring::GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const
{
	const FVector StartWorld = this->GetComponentTransform().TransformPosition(LinePoints[0]);
	const FVector EndWorld = this->GetComponentTransform().TransformPosition(LinePoints[1]);
	const FVector LineDir  = EndWorld-StartWorld;


	const FVector CrossProd = UKismetMathLibrary::Cross_VectorVector(LineDir, Direction); //v
	const FVector LineDifference = StartWorld - Point; //u

	//Project v onto u =>
	const FVector Proj = LineDifference.ProjectOnTo(CrossProd);

	const FVector OffsetPoint = Point + Proj;

	const FVector FromA = OffsetPoint;
	const FVector FromB = StartWorld;
	const FVector ToA = OffsetPoint + Direction * 10000;
	const FVector ToB = EndWorld;
	FVector Result;
	LineToLineIntersection(FromA, FromB, ToA, ToB, Result);

	const FVector LineDirRes = Result - StartWorld;
	if(LineDirRes.Size() > LineDir.Size())
	{
		Result = EndWorld;
	}

	if(!LineDirRes.GetSafeNormal().Equals(LineDir.GetSafeNormal()))
	{
		Result = StartWorld;
	}

	return Result;
}

void UIntenSelectableLineScoring::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(DrawDebug)
	{
		const FVector StartWorld = this->GetComponentTransform().TransformPosition(LinePoints[0]);
		const FVector EndWorld = this->GetComponentTransform().TransformPosition(LinePoints[1]);
		DrawDebugLine(GetWorld(), StartWorld, EndWorld, FColor::Green, false, -1, 0, 2);
	}
}
