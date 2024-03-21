#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCubeScoring.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UIntenSelectableCubeScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA,
									   const FVector& ToB, FVector& OutIntersection);

	FVector GetClosestSelectionPointTo(const FVector& RayOrigin, const FVector& RayDirection) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableCubeScoring();

	UPROPERTY(EditAnywhere)
	bool DrawDebug = true;

	UPROPERTY(EditAnywhere)
	bool BackFaceCulling = false;

	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin,
														   const FVector& ConeForwardDirection,
														   const float ConeBackwardShiftDistance, const float ConeAngle,
														   const float LastValue, const float DeltaTime) override;

	FVector GetClosestPointToRectangle(const FVector& StartPoint, const FVector& Direction, const FVector& Corner00,
									   const FVector& Corner01, const FVector& Corner10) const;
};
