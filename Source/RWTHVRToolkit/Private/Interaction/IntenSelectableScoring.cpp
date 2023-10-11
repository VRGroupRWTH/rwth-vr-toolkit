// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/IntenSelectableScoring.h"

float UIntenSelectableScoring::GetScore(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
	const float ConeBackwardShiftDistance, const float ConeAngle, const FVector& TestPoint, const float LastValue,
	const float DeltaTime)
{
	const FVector ShiftedConeOrigin = ConeOrigin - (ConeForwardDirection * ConeBackwardShiftDistance);
	
	const float D_Perspective = FMath::PointDistToLine(TestPoint, ConeForwardDirection, ShiftedConeOrigin);
	const float D_Projection = (TestPoint - ShiftedConeOrigin).ProjectOnTo(ConeForwardDirection).Size();
	
	const float Angle = FMath::RadiansToDegrees(FMath::Atan(D_Perspective / (FMath::Pow(D_Projection / 100, CompensationConstant) * 100)));
	float S_Contrib = 1 - (Angle / ConeAngle);
	if(S_Contrib < 0) S_Contrib = 0;

	//GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, GetOwner()->GetName() + " - Contrib: " + FString::FromInt(S_Contrib));

	if(LastValue != 0)
	{
		constexpr float Interpolate = 0.5;
		if(S_Contrib > LastValue)
		{
			CurrentScore = LastValue + (((LastValue * Interpolate) + (S_Contrib * (1-Interpolate))) - LastValue) * DeltaTime * Snappiness;
		}else
		{
			CurrentScore = LastValue + (((LastValue * Interpolate) + (S_Contrib * (1-Interpolate))) - LastValue) * DeltaTime * Stickiness;
		}
	}else
	{
		CurrentScore = S_Contrib * Snappiness * DeltaTime;
	}
	return CurrentScore;
}

// Sets default values for this component's properties
UIntenSelectableScoring::UIntenSelectableScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FHitResult, float> UIntenSelectableScoring::GetBestPointScorePair(const FVector& ConeOrigin,
                                                                     const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
                                                                     const float LastValue, const float DeltaTime)
{
	return {};
}

void UIntenSelectableScoring::BeginPlay()
{
	Super::BeginPlay();
}

