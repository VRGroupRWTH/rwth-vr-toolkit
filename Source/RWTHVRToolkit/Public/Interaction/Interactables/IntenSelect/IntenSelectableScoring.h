#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IntenSelectableScoring.generated.h"

/**
 * A component for calculating and managing scores for selectable objects.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableScoring : public USceneComponent
{
	GENERATED_BODY()

protected:
	/**
	 * Calculates a score based on the position and orientation of a cone, and a test point within the environment.
	 *
	 * @param ConeOrigin The origin point of the cone.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backwards.
	 * @param ConeAngle The angle of the cone in degrees.
	 * @param TestPoint The point in the environment to test against the cone.
	 * @param LastValue The last score value calculated for this point.
	 * @param DeltaTime The time elapsed since the last score calculation.
	 * @return The calculated score for the test point.
	 */
	float GetScore(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
				   const float ConeBackwardShiftDistance, const float ConeAngle, const FVector& TestPoint,
				   const float LastValue, const float DeltaTime);

public:
	UIntenSelectableScoring();

	/** Whether to draw debug information in the editor. */
	UPROPERTY(EditAnywhere)
	bool DrawDebug = true;

	/** The current score of the component. */
	UPROPERTY(BlueprintReadOnly)
	float CurrentScore = 0;

	/** Whether the component is selectable. */
	UPROPERTY(EditAnywhere)
	bool bIsSelectable = true;

	/** The stickiness of selection. Higher values make selection more 'sticky'. */
	UPROPERTY(EditAnywhere)
	float Stickiness = 10;

	/** The snappiness of selection. Higher values make selection 'snap' more aggressively. */
	UPROPERTY(EditAnywhere)
	float Snappiness = 15;

	/** A constant used for compensating the score calculation. */
	UPROPERTY(EditAnywhere)
	float CompensationConstant = 0.8;

	/** Whether to overwrite the contribution of this component towards the final score. */
	bool bOverwritingContrib = false;

	/** The contribution of this component towards the final score. */
	float Contrib = 0;

	/**
	 * Gets the best point and score pair based on the input parameters.
	 *
	 * @param ConeOrigin The origin point of the cone.
	 * @param ConeForwardDirection The forward direction of the cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backwards.
	 * @param ConeAngle The angle of the cone in degrees.
	 * @param LastValue The last score value calculated for this point.
	 * @param DeltaTime The time elapsed since the last score calculation.
	 * @return A pair containing the hit result and the score for the best point.
	 */
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime);

	virtual void BeginPlay() override;
};
