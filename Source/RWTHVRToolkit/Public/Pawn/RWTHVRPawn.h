#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "Pawn/Navigation/CollisionHandlingMovement.h"

#include "RWTHVRPawn.generated.h"


class ULiveLinkTrackingComponent;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UMotionControllerComponent;
class UClusterSetupComponent;
struct FLiveLinkTransformStaticData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScaleChangedDelegate, FVector, OldScale, float, NewUniformScale);

/**
 * Pawn implementation with additional VR functionality, can be used in the Cave, with an HMD and on desktop.
 */
UCLASS(Abstract)
class RWTHVRTOOLKIT_API ARWTHVRPawn : public APawn
{
	GENERATED_BODY()

public:
	ARWTHVRPawn(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaSeconds) override;

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
	UClusterSetupComponent* ClusterSetupComponent;

	// LiveLink functionality

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn")
	ULiveLinkTrackingComponent* LiveLinkTrackingComponent;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	TArray<FLiveLinkSubjectRepresentation> HeadSubjectRepresentations;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	TArray<FLiveLinkSubjectRepresentation> LeftSubjectRepresentations;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pawn|LiveLink")
	TArray<FLiveLinkSubjectRepresentation> RightSubjectRepresentations;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void AddInputMappingContext(const APlayerController* PC, const UInputMappingContext* Context) const;

	UFUNCTION(BlueprintCallable)
	UInputComponent* GetPlayerInputComponent();

	/* Fixes camera rotation in desktop mode. */
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction() const;

	/* Set device specific motion controller sources (None, L/R, Livelink) */
	void SetupMotionControllerSources();

	/* Set up livelink sources and tracked component */
	void SetupLiveLinkTracking();

	/* Update viewpoint User Index CVar Callback */
	void SetViewpointUser(IConsoleVariable* Var = nullptr);

private:
	UInputComponent* ActivePlayerInputComponent;
	float InitialWorldToMeters;
	float UniformScale;
};
