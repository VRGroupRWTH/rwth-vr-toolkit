#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableRectangleScoring.generated.h"

/**
 * A component for calculating and managing scores for selectable objects within a rectangular area.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableRectangleScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Determines if two lines intersect, which is particularly useful in calculating
	 * selection boundaries and points within the rectangle.
	 *
	 * @param FromA Starting point of the first line.
	 * @param FromB Ending point of the first line.
	 * @param ToA Starting point of the second line.
	 * @param ToB Ending point of the second line.
	 * @param OutIntersection Output parameter filled with the intersection point, if any.
	 * @return true if the lines intersect, false otherwise.
	 */
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	/**
	 * Calculates the closest point within the rectangle to a given ray, which is essential
	 * for determining selection scores based on the geometry of rectangular areas.
	 *
	 * @param Point The origin point of the ray.
	 * @param Direction The direction of the ray.
	 * @return The closest point within the rectangle to the specified ray.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableRectangleScoring();

	/** If true, only the outline of the rectangle is considered for selection scoring. */
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	/** The length of the rectangle along the X-axis. */
	UPROPERTY(EditAnywhere)
	float XLength = 100;

	/** The length of the rectangle along the Y-axis. */
	UPROPERTY(EditAnywhere)
	float YLength = 100;

	/**
	 * Calculates the best point and score pair for selection within a rectangular area,
	 * taking into account the unique properties and dimensions of the rectangle.
	 *
	 * @param ConeOrigin Origin point of the cone used for selection.
	 * @param ConeForwardDirection Forward direction of the cone used for selection.
	 * @param ConeBackwardShiftDistance Distance to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle Angle of the cone in degrees, used to define the selection area.
	 * @param LastValue Last score value calculated, useful for continuous selection logic.
	 * @param DeltaTime Time elapsed since the last score calculation.
	 * @return A pair containing the hit result and the score for the best selection point within the rectangle.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, float ConeAngle,
														   const float LastValue, const float DeltaTime) override;
};
