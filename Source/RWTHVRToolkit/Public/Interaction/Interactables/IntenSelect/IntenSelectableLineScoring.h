#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableLineScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectableLineScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA, const FVector& ToB, FVector& OutIntersection);
	
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableLineScoring();

	UPROPERTY(EditAnywhere)
	bool DrawDebug = true;
	
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) override;

	UPROPERTY(EditAnywhere, meta=(EditFixedSize))
		TArray<FVector> LinePoints{FVector::ZeroVector, FVector::ZeroVector};
};
