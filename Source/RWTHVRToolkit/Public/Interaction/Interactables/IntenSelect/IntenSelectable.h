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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectable : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSelectable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UIntenSelectableScoring*> ScoringBehaviours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UHoverBehaviour*> OnHoverBehaviours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UActionBehaviour*> OnActionBehaviours;


public:
	UIntenSelectable();

	TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection,
												   const float ConeBackwardShiftDistance, const float ConeAngle,
												   const float LastValue, const float DeltaTime) const;

	void HandleOnHoverStartEvents(const UIntenSelectComponent* IntenSelect, const FHitResult& HitResult);
	void HandleOnHoverEndEvents(const UIntenSelectComponent* IntenSelect);
	void HandleOnActionStartEvents(UIntenSelectComponent* IntenSelect);
	void HandleOnActionEndEvents(UIntenSelectComponent* IntenSelect, FInputActionValue& InputValue);

	void InitDefaultBehaviourReferences();

protected:
	virtual void BeginPlay() override;
};
