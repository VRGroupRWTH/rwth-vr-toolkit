#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableCubeScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectableCubeScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	static bool LineToLineIntersection(const FVector& FromA, const FVector& FromB, const FVector& ToA, const FVector& ToB, FVector& OutIntersection);
	
	FVector GetClosestSelectionPointTo(const FVector& RayOrigin, const FVector& RayDirection);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UIntenSelectableCubeScoring();

	UPROPERTY(EditAnywhere)
	bool DrawDebug = true;

	UPROPERTY(EditAnywhere)
	bool BackFaceCulling = false;
	
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;

	//UPROPERTY(EditAnywhere)
	//float XLength = 100;
	
	//UPROPERTY(EditAnywhere)
	//float YLength = 100;

	//UPROPERTY(EditAnywhere)
	//float ZLength = 100;
	
	virtual TPair<FHitResult, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) override;

	FVector GetClosestPointToRectangle(const FVector& StartPoint, const FVector& Direction, const FVector& Corner00, const FVector& Corner01, const FVector& Corner10, const FVector& Corner11) const;
};
