// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "AI/RVOAvoidanceInterface.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"
#include "Runtime/AIModule/Classes/Navigation/CrowdAgentInterface.h"

#include "RWTHVRPawn.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UMotionControllerComponent;
struct FLiveLinkTransformStaticData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScaleChangedDelegate, FVector, OldScale, float, NewUniformScale);

/**
 * Pawn implementation with additional VR functionality, can be used in the Cave, with an HMD and on desktop.
 */
UCLASS(Abstract)
class RWTHVRTOOLKIT_API ARWTHVRPawn : public APawn, public ICrowdAgentInterface, public IRVOAvoidanceInterface
{
	GENERATED_BODY()

public:
	ARWTHVRPawn(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyControllerChanged() override;

	UFUNCTION(BlueprintCallable)
	void SetScale(float NewScale);

	UFUNCTION(BlueprintCallable)
	float GetScale();

	UPROPERTY(BlueprintAssignable)
	FOnScaleChangedDelegate OnScaleChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Input")
	TArray<UInputMappingContext*> InputMappingContexts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|MotionControllers")
	UMotionControllerComponent* RightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|MotionControllers")
	UMotionControllerComponent* LeftHand;

	/* Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Movement")
	UCollisionHandlingMovement* CollisionHandlingMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Desktop Movement")
	bool bMoveRightHandWithMouse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn|Desktop Movement")
	bool bMoveLeftHandWithMouse = false;

	/* CameraComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn|Camera")
	UCameraComponent* HeadCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn")
	USceneComponent* SyncComponent;

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
	
	// ICrowdAgentInterface Implementation
	// These functions allow the Detour Crowd to "see" your Pawn
	// ICrowdAgentInterface Implementation
	virtual FVector GetCrowdAgentLocation() const override;
	virtual FVector GetCrowdAgentVelocity() const override;
	virtual void GetCrowdAgentCollisions(float& CylinderRadius, float& CylinderHalfHeight) const override;
	virtual float GetCrowdAgentMaxSpeed() const override;
	virtual int32 GetCrowdAgentAvoidanceGroup() const override;
	virtual int32 GetCrowdAgentGroupsToAvoid() const override;
	virtual int32 GetCrowdAgentGroupsToIgnore() const override;
	
	// IRVOAvoidance Implementation
	virtual void SetRVOAvoidanceUID(int32 UID) override;
	virtual int32 GetRVOAvoidanceUID() override;
	virtual void SetRVOAvoidanceWeight(float Weight) override;
	virtual float GetRVOAvoidanceWeight() override;
	virtual FVector GetRVOAvoidanceOrigin() override;
	virtual float GetRVOAvoidanceRadius() override;
	virtual float GetRVOAvoidanceHeight() override;
	virtual float GetRVOAvoidanceConsiderationRadius() override;
	virtual FVector GetVelocityForRVOConsideration() override;
	virtual void SetAvoidanceGroupMask(int32 GroupFlags) override;
	virtual int32 GetAvoidanceGroupMask() override;
	virtual void SetGroupsToAvoidMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToAvoidMask() override;
	virtual void SetGroupsToIgnoreMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToIgnoreMask() override;
	FVector GetActorFeetLocation() const;

	/** No default value, for now it's assumed to be valid if GetAvoidanceManager() returns non-NULL. **/
	UPROPERTY(Category="Character Movement: Avoidance", VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 AvoidanceUID;
	
	/** De facto default value 0.5 (due to that being the default in the avoidance registration function), indicates RVO behavior. */
	UPROPERTY(Category="Character Movement: Avoidance", EditAnywhere, BlueprintReadOnly)
	float AvoidanceWeight;
	
	UPROPERTY(Category="Character Movement: Avoidance", EditAnywhere, BlueprintReadOnly, meta=(ForceUnits=cm))
	float AvoidanceConsiderationRadius;
	
	/** Current velocity of updated component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Velocity)
	FVector Velocity;
	
	/** Moving actor's group mask */
	UPROPERTY(Category="Character Movement: Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup;
	
	/** Will avoid other agents if they are in one of specified groups */
	UPROPERTY(Category="Character Movement: Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid;
	
	/** Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid */
	UPROPERTY(Category="Character Movement: Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore;
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void AddInputMappingContext(const APlayerController* PC, const UInputMappingContext* Context) const;

	UFUNCTION(BlueprintCallable)
	UInputComponent* GetPlayerInputComponent();

	/* LiveLink helper function called on tick */
	void EvaluateLivelink() const;

	/* Helper function that applies the LiveLink data to this component. Taken from the LiveLink Transform Controller.
	 */
	void ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData) const;

	/* Fixes camera rotation in desktop mode. */
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction() const;

	/* Replicated functionality */

	/* Add a sync component to all instances of this pawn */
	UFUNCTION(Reliable, NetMulticast)
	void MulticastAddDCSyncComponent();

	/* Attaches the Cluster representation to the pawn */
	void AttachClustertoPawn();

	/* Set device specific motion controller sources (None, L/R, Livelink) */
	void SetupMotionControllerSources();
	
	/** Helper to find the capsule component in children */
	class UCapsuleComponent* GetCapsuleComponent() const;

private:
	UInputComponent* ActivePlayerInputComponent;
	float InitialWorldToMeters;
	float UniformScale;
};
