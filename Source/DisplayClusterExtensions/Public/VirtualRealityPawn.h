// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "UniversalTrackedComponent.h"
#include "VRPawnMovement.h"
#include "VirtualRealityPawn.generated.h"

/**
 * 
 */
UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API AVirtualRealityPawn : public APawn
{
	GENERATED_BODY()
public:
	AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer);
	
	/* Proxy Components */
	UPROPERTY() UUniversalTrackedComponent* Head;
	UPROPERTY() UUniversalTrackedComponent* RightHand;
	UPROPERTY() UUniversalTrackedComponent* LeftHand;
	UPROPERTY() UUniversalTrackedComponent* Tracker1;
	UPROPERTY() UUniversalTrackedComponent* Tracker2;
	UPROPERTY() UBasicVRInteractionComponent* BasicVRInteraction;

	/* Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement") UVRPawnMovement* PawnMovement;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement") float BaseTurnRate = 45.0f;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* Movement */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnForward(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnRight(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnTurnRate(float Rate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnLookUpRate(float Rate);

	/* Interaction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Interaction") void OnBeginFire(); 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Interaction") void OnEndFire(); 
};
