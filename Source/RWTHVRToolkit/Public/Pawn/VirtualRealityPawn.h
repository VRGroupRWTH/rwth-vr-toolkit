// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "UniversalTrackedComponent.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* Head;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* RightHand;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects")
	UUniversalTrackedComponent* LeftHand;

	/* Interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Interaction")
	UBasicVRInteractionComponent* BasicVRInteraction;

	/* Movement */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement")
	UVRPawnMovement* PawnMovement;*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement")
	bool bMoveWithRightHand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement")
	bool bSnapTurn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement", meta=(EditCondition="!bSnapTurn"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement", meta=(EditCondition="bSnapTurn",ClampMin=0,ClampMax=360))
	float SnapTurnAngle = 22.5;

	/** Workaround dummy component to prevent the Capsule from rotating in the editor, if LiveLink tracking is being used.
	 *  This happens due to the rotation of the Capsule being set only while in Play Mode (instead of using e.g. absolute rotation).
	 *  Additionally, there is an implicit race condition in Tick, due to LiveLink adjusting the parent's rotation, while the capsule
	 *  then gets rotated back in Tick to be vertical. Depending on the order, LiveLink overrides the VRPawnMovement's rotation settings.
	 *  The dummy seems to fix this, because its absolute rotation just catches all parent rotations and prevents them from
	 *  overriding any of the capsules'.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement")
	USceneComponent* CapsuleRotationFix;


	/* CameraComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Camera")
	UCameraComponent* CameraComponent;


protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* Movement */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Movement")
	void OnUp(const FInputActionValue& Value);

	/* Interaction */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginFire(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnEndFire(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginGrab(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnEndGrab(const FInputActionValue& Value);

	/*
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginMove(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Interaction")
	void OnBeginSnapTurn(const FInputActionValue& Value);
	*/
	
	/* Input */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|Input")
	class UInputMappingContext* IMCBase;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|Input")
	class UVRPawnInputConfig* InputActions;
	
	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	/*UFUNCTION()
	void StartDesktopRotation();
	
	UFUNCTION()
	void EndDesktopRotation();
	
	bool bApplyDesktopRotation = false;*/

	/**
	 * Fixes camera rotation in desktop mode.
	 */
	/*void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();*/

private:
	/*UPROPERTY()
	UUniversalTrackedComponent* MovementHand;
	
	UPROPERTY()
	UUniversalTrackedComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCMovement;*/

	/*virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;*/

};
