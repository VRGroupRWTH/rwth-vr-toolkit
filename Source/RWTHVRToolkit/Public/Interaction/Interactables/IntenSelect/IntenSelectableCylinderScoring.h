#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCylinderScoring.generated.h"

/**
 * A component for calculating and managing scores for selectable objects within a cylindrical area.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableCylinderScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Checks for intersection between two lines, which is useful for calculations involving
	 * the cylinder's geometry and selection logic.
	 *
	 * @param FromA The starting point of the first line.
	 * @param FromB The ending point of the first line.
	 * @param ToA The starting point of the second line.
	 * @param ToB The ending point of the second line.
	 * @param OutIntersection If an intersection exists, this parameter will be filled with the point of intersection.
	 * @return true if there is an intersection; false otherwise.
	 */
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	/**
	 * Determines the closest point on the cylinder to a given ray, which can be used to calculate
	 * selection scores based on proximity and directionality.
	 *
	 * @param Point The origin point of the ray.
	 * @param Direction The direction of the ray.
	 * @return The closest point on the cylinder to the specified ray.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableCylinderScoring();

	/**
	 * Overrides the parent class method to calculate the best point and score pair for selection
	 * within a cylindrical volume, taking into account the unique geometry of a cylinder.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance How far to shift the cone's origin backward for selection.
	 * @param ConeAngle The angle of the cone, in degrees, used for determining the selection area.
	 * @param LastValue The last score value calculated, used for continuous adjustments.
	 * @param DeltaTime Time since the last score calculation.
	 * @return A pair containing the hit result and score for the best selection point.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	/** Defines the end points of the cylinder's central axis, used in selection calculations. */
	UPROPERTY(EditAnywhere)
	TArray<FVector> LinePoints{FVector::UpVector * 50, FVector::DownVector * 50};

	/** The radius of the cylinder, important for determining the cylindrical selection area. */
	UPROPERTY(EditAnywhere)
	float Radius = 50;
};
