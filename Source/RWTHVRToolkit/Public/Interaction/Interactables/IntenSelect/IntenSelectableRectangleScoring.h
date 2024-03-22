#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableRectangleScoring.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableRectangleScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableRectangleScoring();

	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	UPROPERTY(EditAnywhere)
	float XLength = 100;

	UPROPERTY(EditAnywhere)
	float YLength = 100;

	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;
};
