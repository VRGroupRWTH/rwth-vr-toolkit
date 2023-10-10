#include "Interaction/IntenSelectableSinglePointScoring.h"

UIntenSelectableSinglePointScoring::UIntenSelectableSinglePointScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectableSinglePointScoring::GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance,
	const float ConeAngle, const float LastValue, const float DeltaTime)
{
	const FVector TestPoint = this->GetComponentLocation();
	float Score = GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, TestPoint, LastValue, DeltaTime);
	return TPair<FVector, float>{TestPoint, Score};
}

void UIntenSelectableSinglePointScoring::BeginPlay()
{
	Super::BeginPlay();
}

