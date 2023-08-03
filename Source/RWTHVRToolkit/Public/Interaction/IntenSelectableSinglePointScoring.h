#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableSinglepointScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectableSinglePointScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

public:
	UIntenSelectableSinglePointScoring();
	
	virtual TPair<FVector, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) override;

	virtual void BeginPlay() override;
};
