// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IntenSelectableScoring.h"
#include "IntenSelectable.generated.h"


class UClickBehaviour;
class USelectionBehaviour;
class UIntenSelectComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectable : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSelectable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UIntenSelectableScoring* ScoringBehaviour;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UHoverBehaviour*> OnSelectBehaviours;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UClickBehaviour*> OnClickBehaviours;
	

public:
	UIntenSelectable();

	TPair<FVector, float> GetBestPointScorePair(const FVector& ConeOrigin, const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle, const float LastValue, const float DeltaTime) const;
	
	void HandleOnSelectStartEvents(const UIntenSelectComponent* IntenSelect, const FVector& Point);
	void HandleOnSelectEndEvents(const UIntenSelectComponent* IntenSelect);
	void HandleOnClickStartEvents(const UIntenSelectComponent* IntenSelect, const FVector& Point);
	void HandleOnClickEndEvents(const UIntenSelectComponent* IntenSelect);

	void InitDefaultBehaviourReferences();

	void ShowErrorAndQuit(const FString& Message) const;

protected:
	virtual void BeginPlay() override;
};
