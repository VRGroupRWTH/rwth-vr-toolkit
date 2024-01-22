#include "Interaction/Interactables/IntenSelect/IntenSelectableSinglePointScoring.h"

UIntenSelectableSinglePointScoring::UIntenSelectableSinglePointScoring()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FHitResult, float> UIntenSelectableSinglePointScoring::GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance,
	const float ConeAngle, const float LastValue, const float DeltaTime)
{
	const FVector Point = this->GetComponentLocation();
	float Score = GetScore(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, Point, LastValue, DeltaTime);
	FHitResult Result = FHitResult{GetOwner(), nullptr, Point, FVector::ForwardVector};
	return TPair<FHitResult, float>{Result, Score};
}

void UIntenSelectableSinglePointScoring::BeginPlay()
{
	Super::BeginPlay();
}

