// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "UniversalTrackedComponent.h"
#include "VRPawnMovement.h"
#include "VirtualRealityPawn.generated.h"

class ULiveLinkComponentController;
/**
 * 
 */
UCLASS()
class RWTHVRTOOLKIT_API AVirtualRealityPawn : public APawn
{
	GENERATED_BODY()
public:
	AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer);
	
	/* Proxy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* Head;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* RightHand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Proxy Objects") UUniversalTrackedComponent* LeftHand;

	/* Interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Interaction") UBasicVRInteractionComponent* BasicVRInteraction;

	/* Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement") UVRPawnMovement* PawnMovement;

	/** Workaround dummy component to prevent the Capsule from rotating in the editor, if LiveLink tracking is being used.
	 *  This happens due to the rotation of the Capsule being set only while in Play Mode (instead of using e.g. absolute rotation).
	 *  Additionally, there is an implicit race condition in Tick, due to LiveLink adjusting the parent's rotation, while the capsule
	 *  then gets rotated back in Tick to be vertical. Depending on the order, LiveLink overrides the VRPawnMovement's rotation settings.
	 *  The dummy seems to fix this, because its absolute rotation just catches all parent rotations and prevents them from
	 *  overriding any of the capsules'.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement") USceneComponent* CapsuleRotationFix;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Movement") float BaseTurnRate = 45.0f;

	/* CameraComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Camera") UCameraComponent* CameraComponent;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* Movement */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnForward(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnRight(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnUp(float Value);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnTurnRate(float Rate);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Movement") void OnLookUpRate(float Rate);

	/* Interaction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Interaction") void OnBeginFire(); 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn|Interaction") void OnEndFire();

	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION() void StartDesktopRotation();
	UFUNCTION() void EndDesktopRotation();
	bool bApplyDesktopRotation = false;

	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();
};
