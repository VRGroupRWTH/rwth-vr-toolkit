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

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if(bOverwritingContrib)
	{
		S_Contrib = Contrib;
	}
#endif

	if(LastValue != 0)
	{
		CurrentScore = LastValue * Stickiness + S_Contrib * Snappiness * DeltaTime;
		//CurrentScore = ((LastValue - ((1 - Stickiness) * LastValue * DeltaTime) + (S_Contrib * Snappiness *  DeltaTime))) / 2;
		//CurrentScore = ((LastValue - ((1 - Stickiness) * LastValue * DeltaTime * 100) + (S_Contrib * DeltaTime * 100))) / 2;
		//CurrentScore = LastValue + ((((LastValue * Stickiness) + (S_Contrib * ((1-Stickiness)))) - LastValue)) * DeltaTime * 10;

		constexpr float interpolate = 0.5;
		if(S_Contrib > LastValue)
		{
			CurrentScore = LastValue + ((LastValue * interpolate + S_Contrib * (1-interpolate)) - LastValue) * DeltaTime * Snappiness;
		}else
		{
			CurrentScore = LastValue + ((LastValue * interpolate + S_Contrib * (1-interpolate)) - LastValue) * DeltaTime * Stickiness;
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

void UIntenSelectableScoring::SetContrib(float Value)
{
	bOverwritingContrib = true;
	this->Contrib = Value;	
}

void UIntenSelectableScoring::ClearContrib()
{
	bOverwritingContrib = false;
	this->Contrib = 0;
}

TPair<FVector, float> UIntenSelectableScoring::GetBestPointScorePair(const FVector& ConeOrigin,
                                                                     const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
                                                                     const float LastValue, const float DeltaTime)
{
	return {};
}

void UIntenSelectableScoring::BeginPlay()
{
	Super::BeginPlay();
}

