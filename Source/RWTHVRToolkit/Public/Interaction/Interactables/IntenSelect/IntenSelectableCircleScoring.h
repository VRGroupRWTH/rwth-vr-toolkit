#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCircleScoring.generated.h"

/**
 * A component for calculating and managing scores for selectable objects within a circular area.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableCircleScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Determines the closest point within a circle to a given point, taking into account the direction of selection.
	 *
	 * @param Point The point from which to find the closest selection point.
	 * @param Direction The direction in which the selection is being made.
	 * @return The closest point within the circle to the specified point, considering the given direction.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableCircleScoring();

	/** If true, selection scoring is only considered on the outline of the circle. Otherwise, the entire area is
	 * considered. */
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = true;

	/** The radius of the circle used for selection scoring. */
	UPROPERTY(EditAnywhere)
	float Radius = 50;

	/**
	 * Overrides the parent class's method to calculate the best point and score pair for selection within a circular
	 * area.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone used for selection.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backwards, for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, used for selection.
	 * @param LastValue The last score value calculated for this point, used for selection.
	 * @param DeltaTime The time elapsed since the last score calculation, used for selection.
	 * @return A pair containing the hit result and the score for the best point within the circular area.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;
};
