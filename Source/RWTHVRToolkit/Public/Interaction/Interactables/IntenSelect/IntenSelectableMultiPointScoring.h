#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableMultiPointScoring.generated.h"

/**
 * A component that specializes in calculating and managing selection scores based on multiple points.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableMultiPointScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Calculates the closest selection point to a given reference point, factoring in a specific direction.
	 * This function is crucial for multi-point selection scenarios, where the closest point
	 * within a set needs to be determined based on proximity and direction.
	 *
	 * @param Point The reference point from which to find the closest selection point.
	 * @param Direction The direction from the reference point towards the selection points.
	 * @return The closest point from the provided set to the specified reference point and direction.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableMultiPointScoring();

	/**
	 * Overrides the base class method to calculate the best point and score pair from among multiple points,
	 * based on the specified selection criteria.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, defining the selection area.
	 * @param LastValue The last score value calculated, for continuous adjustments of the selection score.
	 * @param DeltaTime Time elapsed since the last score calculation.
	 * @return A pair containing the hit result and the score for the best point among the multiple points.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	/**
	 * Updates the set of points to be considered for selection. This can be used to dynamically
	 * change the points that are eligible for selection, based on gameplay or application logic.
	 */
	void UpdatePoints();

	/** The set of points to consider for selection. These points are evaluated to find the best selection score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PointsToSelect;
};
