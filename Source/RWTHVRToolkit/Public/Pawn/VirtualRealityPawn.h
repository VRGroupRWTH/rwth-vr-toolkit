// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "Pawn/Navigation/VRPawnMovement.h"
#include "VirtualRealityPawn.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UMotionControllerComponent;
struct FLiveLinkTransformStaticData;

/**
 * Pawn implementation with additional VR functionality, can be used in the Cave, with an HMD and on desktop.
 */
UCLASS(Abstract)
class RWTHVRTOOLKIT_API AVirtualRealityPawn : public APawn
{
	GENERATED_BODY()

public:
	AVirtualRealityPawn(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyControllerChanged() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|MotionControllers")
	UMotionControllerComponent* RightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|MotionControllers")
	UMotionControllerComponent* LeftHand;

	/* Interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Interaction")
	UBasicVRInteractionComponent* BasicVRInteraction;

	/* Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement")
	UVRPawnMovement* PawnMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Desktop Movement")
	bool bMoveRightHandWithMouse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Desktop Movement")
	bool bMoveLeftHandWithMouse = false;

	/* CameraComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Camera")
	UCameraComponent* HeadCameraComponent;

	// LiveLink functionality

	/* Set whether nDisplay should disable LiveLink tracking*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|LiveLink")
	bool bDisableLiveLink = false;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation HeadSubjectRepresentation;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation LeftSubjectRepresentation;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation RightSubjectRepresentation;

	/* Set the transform of the component in world space of in its local reference frame. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	bool bWorldTransform = false;

	/* The class which to search for DCRA attachment. TODO: Make this better it's ugly */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	TSubclassOf<AActor> CaveSetupActorClass;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* LiveLink helper function called on tick */
	void EvaluateLivelink() const;

	/* Helper function that applies the LiveLink data to this component. Taken from the LiveLink Transform Controller. */
	void ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData) const;

	/* Fixes camera rotation in desktop mode. */
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction() const;

	/* Replicated functionality */

	/* Ask the server to attach the DCRA to the correct pawn */
	UFUNCTION(Reliable, Server)
	void ServerAttachDCRAtoPawnRpc();

	/* Attaches the DCRA to the pawn */
	void AttachDCRAtoPawn();

	/* Set device specific motion controller sources (None, L/R, Livelink) */
	void SetupMotionControllerSources();
};
