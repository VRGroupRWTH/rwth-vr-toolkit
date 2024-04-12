#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCubeScoring.generated.h"

/**
 * A component that specializes in calculating and managing selection scores within a cubic area.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableCubeScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Determines if two lines intersect, which is critical for calculating selection boundaries
	 * and interactions within or around the cube.
	 *
	 * @param FromA The starting point of the first line.
	 * @param FromB The ending point of the first line.
	 * @param ToA The starting point of the second line.
	 * @param ToB The ending point of the second line.
	 * @param OutIntersection Output parameter that will be filled with the intersection point if an intersection
	 * occurs.
	 * @return true if the lines intersect; otherwise, false.
	 */
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	/**
	 * Calculates the closest point within the cube to a given ray, which is essential for selection scoring.
	 *
	 * @param RayOrigin The origin of the ray.
	 * @param RayDirection The direction of the ray.
	 * @return The closest point within the cube to the specified ray.
	 */
	FVector GetClosestSelectionPointTo(const FVector& RayOrigin, const FVector& RayDirection) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableCubeScoring();

	/** Whether back faces of the cube should be considered in the selection process. */
	UPROPERTY(EditAnywhere)
	bool BackFaceCulling = false;

	/** If true, selection scoring is only considered on the outline of the cube, not the volume. */
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	/**
	 * Overrides the base class method to compute the best point and score pair for selection within a cubic volume,
	 * considering the unique properties and geometry of the cube.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance How far to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, used to define the selection area.
	 * @param LastValue The last score value calculated, useful for continuous adjustments of the selection score.
	 * @param DeltaTime Time since the last score calculation.
	 * @return A pair containing the hit result and the score for the best point within the cubic volume.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	/**
	 * Finds the closest point on a rectangle, part of the cube, to a given start point, projecting in a specified
	 * direction. This function is useful for determining interaction points on the cube's surface.
	 *
	 * @param StartPoint The point from which closeness is measured.
	 * @param Direction The direction in which the closeness is projected.
	 * @param Corner00 One corner of the rectangle.
	 * @param Corner01 Adjacent corner to Corner00.
	 * @param Corner10 Adjacent corner to Corner00, opposite to Corner01.
	 * @return The closest point on the rectangle to the given start point.
	 */
	FVector GetClosestPointToRectangle(const FVector& StartPoint, const FVector& Direction, const FVector& Corner00,
									   const FVector& Corner01, const FVector& Corner10) const;
};
