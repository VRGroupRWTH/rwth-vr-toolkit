#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableSinglePointScoring.generated.h"

/**
 * A component specifically designed for calculating and managing scores for selectable objects at a single point.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableSinglePointScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

public:
	UIntenSelectableSinglePointScoring();

	/**
	 * Overrides the base class method to calculate the best point and score pair for a single point selection.
	 * This method is crucial for determining the effectiveness of selection based on the specified criteria.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, defining the selection area.
	 * @param LastValue The last score value calculated, for continuous adjustments of the selection score.
	 * @param DeltaTime Time elapsed since the last score calculation.
	 * @return A pair containing the hit result and the score for the best selection point, which in this case is
	 * predetermined.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	virtual void BeginPlay() override;
};
