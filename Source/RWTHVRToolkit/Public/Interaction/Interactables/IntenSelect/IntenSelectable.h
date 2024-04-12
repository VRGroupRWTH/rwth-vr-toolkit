#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "Interaction/Interactables/ActionBehaviour.h"
#include "Interaction/Interactables/HoverBehaviour.h"
#include "IntenSelectable.generated.h"

class UIntenSelectableScoring;
class UClickBehaviour;
class USelectionBehaviour;
class UIntenSelectComponent;

/**
 * A component designed to make an actor selectable within IntenSelect, handling various aspects
 * of interaction including scoring, hover, and action behaviors.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectable : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Whether the actor can be selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSelectable = true;

	/** Collection of scoring behaviors that define how selection scores are calculated for this actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UIntenSelectableScoring*> ScoringBehaviours;

	/** Collection of behaviors that define what happens when this actor is hovered over. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UHoverBehaviour*> OnHoverBehaviours;

	/** Collection of behaviors that define what happens when an action is performed on this actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UActionBehaviour*> OnActionBehaviours;

	UIntenSelectable();

	/**
	 * Computes the best point and score for selection based on defined criteria and behaviors.
	 *
	 * @param ConeOrigin The origin point of the selection cone.
	 * @param ConeForwardDirection The forward direction of the selection cone.
	 * @param ConeBackwardShiftDistance The distance to shift the cone's origin backwards.
	 * @param ConeAngle The angle of the selection cone.
	 * @param LastValue The last score value calculated for ongoing adjustments.
	 * @param DeltaTime Time elapsed since the last score calculation.
	 * @return A pair containing the best hit result and its corresponding score.
	 */
	TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
												   const float ConeBackwardShiftDistance, const float ConeAngle,
												   const float LastValue, const float DeltaTime) const;

	/** Handles events when hover starts over this actor. */
	void HandleOnHoverStartEvents(const UIntenSelectComponent* IntenSelect, const FHitResult& HitResult);

	/** Handles events when hover ends over this actor. */
	void HandleOnHoverEndEvents(const UIntenSelectComponent* IntenSelect);

	/** Handles events when an action starts on this actor. */
	void HandleOnActionStartEvents(UIntenSelectComponent* IntenSelect);

	/** Handles events when an action ends on this actor. */
	void HandleOnActionEndEvents(UIntenSelectComponent* IntenSelect, FInputActionValue& InputValue);

	/** Initializes default references for the component's behaviors. Useful for setting up initial states. */
	void InitDefaultBehaviourReferences();

protected:
	virtual void BeginPlay() override;
};
