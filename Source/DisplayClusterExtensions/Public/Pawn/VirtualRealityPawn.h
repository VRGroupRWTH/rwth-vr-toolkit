// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
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
	
	/* Proxy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* Head;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* RightHand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* LeftHand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* Tracker1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* Tracker2;

	/* Interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Interaction") UBasicVRInteractionComponent* BasicVRInteraction;

	/* Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement") UVRPawnMovement* PawnMovement;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement") float BaseTurnRate = 45.0f;

	/* CameraComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Camera") UCameraComponent* CameraComponent;
	
protected:
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
