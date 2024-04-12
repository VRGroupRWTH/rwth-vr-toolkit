#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableSphereScoring.generated.h"

/**
 * A component designed for calculating and managing selection scores within a spherical area.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableSphereScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	/**
	 * Determines the closest point within the sphere to a given point and direction, which is essential
	 * for calculating selection scores that are based on proximity to a spherical surface or volume.
	 *
	 * @param Point The reference point from which to find the closest point on the sphere.
	 * @param Direction The direction from the reference point to project towards the sphere.
	 * @return The closest point on the sphere to the given reference point and direction.
	 */
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableSphereScoring();

	/**
	 * Optional debug material to visualize the selection scoring area in the Unreal Editor or during gameplay.
	 * This material can be applied to a sphere mesh to aid in debugging and visualizing the scoring volume.
	 */
	UPROPERTY(EditAnywhere)
	UMaterialInstance* DebugMaterial;

	/**
	 * Determines whether the scoring calculation should only consider the outline of the sphere.
	 * If true, only the surface area of the sphere is considered for scoring. Otherwise, the entire
	 * volume of the sphere is used in the scoring calculations.
	 */
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	/** The radius of the sphere used in the selection scoring calculations. */
	UPROPERTY(EditAnywhere)
	float Radius = 50;

	/**
	 * Overrides the base class method to calculate the best point and score pair for selections
	 * within a spherical volume, taking into account the sphere's radius and whether to consider
	 * just the outline or the entire volume for scoring.
	 *
	 * @param ConeOrigin The origin point of the cone used for selection.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance How far to shift the cone's origin backward for selection purposes.
	 * @param ConeAngle The angle of the cone in degrees, used to determine the selection area.
	 * @param LastValue The last score value calculated, for continuous adjustment of selection scoring.
	 * @param DeltaTime Time since the last score calculation.
	 * @return A pair containing the hit result and the score for the best point within or on the surface of the sphere.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;
};
