#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableSinglePointScoring.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableSinglePointScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

public:
	UIntenSelectableSinglePointScoring();

	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	virtual void BeginPlay() override;
};
