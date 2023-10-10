// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectableSphereScoring.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectableSphereScoring : public UIntenSelectableScoring
{
	GENERATED_BODY()

protected:
	FVector GetClosestSelectionPointTo(const FVector& Point, const FVector& Direction) const;

public:
	UIntenSelectableSphereScoring();

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DebugMaterial;
	
	UPROPERTY(EditAnywhere)
	bool OnlyOutline = false;
	
	UPROPERTY(EditAnywhere)
	float Radius = 50;
	
	UPROPERTY(EditAnywhere)
	bool DrawDebug = false;
	
	virtual TPair<FVector, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) override;

};
