#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IntenSelectableScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableScoring : public USceneComponent
{
	GENERATED_BODY()

protected:
	float GetScore(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const FVector& TestPoint, const float LastValue, const float DeltaTime);
	
public:
	UIntenSelectableScoring();

	UFUNCTION(BlueprintCallable, meta=(CallableWithoutWorldContext, DevelopmentOnly))
	void SetContrib(float Value);
	UFUNCTION(BlueprintCallable, meta=(CallableWithoutWorldContext, DevelopmentOnly))
	void ClearContrib();

	UPROPERTY(BlueprintReadOnly)
	float CurrentScore = 0;
	UPROPERTY(EditAnywhere)
		bool IsSelectable = true;
	UPROPERTY(EditAnywhere)
		float Stickiness = 0.1;
	UPROPERTY(EditAnywhere)
		float Snappiness = 5;
	UPROPERTY(EditAnywhere)
		float CompensationConstant = 0.8;

	bool bOverwritingContrib = false;
	float Contrib = 0;

	virtual TPair<FVector, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime);

	virtual void BeginPlay() override;
};
