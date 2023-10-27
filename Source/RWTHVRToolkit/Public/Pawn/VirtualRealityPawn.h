// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasicVRInteractionComponent.h"
#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "Pawn/VRPawnMovement.h"
#include "VirtualRealityPawn.generated.h"

class UCameraComponent;
class UMotionControllerComponent;
struct FLiveLinkTransformStaticData;

/**
 * 
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
	
	/* CameraComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Camera")
	UCameraComponent* HeadCameraComponent;

	/** Set whether nDisplay should disable LiveLink tracking*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|LiveLink")
	bool bDisableLiveLink = false;

	/** Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation HeadSubjectRepresentation;

	/** Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation LeftSubjectRepresentation;

	/** Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	FLiveLinkSubjectRepresentation RightSubjectRepresentation;
	
	/** Set the transform of the component in world space of in its local reference frame. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	bool bWorldTransform = false;

	/** The class which to search for DCRA attachment. TODO: Make this better it's ugly */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	TSubclassOf<AActor> CaveSetupActorClass;
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void EvaluateLivelink();
	/**
	 * @brief Helper function that applies the LiveLink data to this component. Taken from the LiveLink Transform Controller.
	 * @param Transform Transform read from the LiveLink Client.
	 * @param StaticData Static data from the LiveLink Subject, defines what is and isn't supported.
	 * @return void
	 */
	void ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData);

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

	/**
	* Fixes camera rotation in desktop mode.
	*/
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();

	/**
	 * Ask the server to attach the DCRA to the correct pawn
	 * @return void
	 */
	UFUNCTION(Reliable, Server)
	void ServerAttachDCRAtoPawnRpc();

	/**
	 * Attaches the DCRA to the pawn
	 * @return void
	 */
	void AttachDCRAtoPawn();

	void SetupMotionControllerSources();
};
