#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableLineScoring.generated.h"

/**
 * A component that specializes in calculating and managing scores for selectable objects along a line.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableLineScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Checks for the intersection between two lines, which is crucial for determining
	 * the selection points and scoring along lines.
	 *
	 * @param FromA Starting point of the first line.
	 * @param FromB Ending point of the first line.
	 * @param ToA Starting point of the second line.
	 * @param ToB Ending point of the second line.
	 * @param OutIntersection Output parameter that will be populated with the intersection point, if it exists.
	 * @return true if an intersection occurs; false otherwise.
	 */
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	/**
	 * Determines the closest point on the line to a given point, considering a specific direction.
	 * This method is pivotal for line-based selection scoring calculations.
	 *
	 * @param Point The reference point from which to find the closest point on the line.
	 * @param Direction The direction from the reference point towards the line.
	 * @return The closest point on the line to the specified point and direction.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableLineScoring();

	/**
	 * Overrides the parent class's method to calculate the best point and score pair for selection
	 * within a line, accounting for the unique linear geometry.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, defining the selection area.
	 * @param LastValue The last score value calculated, for continuous selection logic adjustments.
	 * @param DeltaTime Time elapsed since the last score calculation.
	 * @return A pair containing the hit result and the score for the best point along the line.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	/**
	 * Defines the endpoints of the line used for selection scoring. The selection logic will
	 * consider these points to calculate the closest point and score along the line.
	 */
	UPROPERTY(EditAnywhere, meta = (EditFixedSize))
	TArray<FVector> LinePoints{FVector::RightVector * 50, FVector::LeftVector * 50};
};
