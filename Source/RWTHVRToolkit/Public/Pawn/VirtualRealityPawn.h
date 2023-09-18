// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "UniversalTrackedComponent.h"
#include "Pawn/VRPawnMovement.h"
#include "VirtualRealityPawn.generated.h"

class UCameraComponent;
class ULiveLinkComponentController;


/**
 * 
 */
UCLASS(Abstract)
class RWTHVRTOOLKIT_API AVirtualRealityPawn : public APawn
{
	GENERATED_BODY()
public:
	AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer);
	
	/* Proxy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* Head;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* RightHand;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* LeftHand;

	/* Interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Interaction")
	UBasicVRInteractionComponent* BasicVRInteraction;
	
	/* Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement")
	UVRPawnMovement* PawnMovement;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement")
	USceneComponent* CapsuleRotationFix;
	
	/* CameraComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Camera")
	UCameraComponent* CameraComponent;


protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* Interaction */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginFire(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnEndFire(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnToggleNavigationMode(const FInputActionValue& Value);
	
	/* Input */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|Input")
	class UInputMappingContext* IMCBase;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|Input")
	class UInputAction* Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|Input")
	class UInputAction* ToggleNavigationMode;

};
