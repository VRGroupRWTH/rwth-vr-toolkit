// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "UniversalTrackedComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversalTrackedComponent, Log, All);

struct FLiveLinkTransformStaticData;

UENUM(BlueprintType)
enum class ETrackedComponentType : uint8
{
	TCT_TRACKER_1 UMETA(DisplayName = "VIVE Tracker 1"),
	TCT_TRACKER_2 UMETA(DisplayName = "VIVE Tracker 2"),
	TCT_RIGHT_HAND UMETA(DisplayName = "Right Hand"),
	TCT_LEFT_HAND UMETA(DisplayName = "Left Hand"),
	TCT_HEAD UMETA(DisplayName = "Head")
};

UENUM(BlueprintType)
enum class EAttachementType : uint8
{
	AT_NONE UMETA(DisplayName = "not attached"),
	AT_HANDTARGET UMETA(DisplayName = "to the right/left hand target"),
	AT_FLYSTICK UMETA(DisplayName = "to the Flystick")
};

/*
 * Acts as an intelligent proxy object, when attached to a Pawn class
 * Attaches itself to the specified controller (Camera, MotionController, TrackedDevice) once they are available
 * Behaves according to the ETrackedComponentType which has to be set before starting
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UUniversalTrackedComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUniversalTrackedComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
	ETrackedComponentType ProxyType = ETrackedComponentType::TCT_HEAD;

	/** Set whether LiveLink should always be enabled, even in the editor or desktop mode. Overrides bUseLiveLinkTracking.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
	bool bAlwaysUseLiveLinkTracking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking|nDisplay")
	EAttachementType AttachementType = EAttachementType::AT_NONE;

	/** Set whether nDisplay should use LiveLink tracking. Requires a valid Subject Representation!*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking|nDisplay")
	bool bUseLiveLinkTracking = true;

	/** Set the LiveLink Subject Representation to be used by this component. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Tracking|nDisplay|LiveLink")
	FLiveLinkSubjectRepresentation SubjectRepresentation;

	/** Set the transform of the component in world space of in its local reference frame. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking|nDisplay|LiveLink")
	bool bWorldTransform = false;

	/** Whether we should set the owning actor's location with the value coming from live link. */
	UPROPERTY(EditAnywhere, Category = "Tracking|nDisplay|LiveLink")
	bool bUseLocation = true;

	/** Whether we should set the owning actor's rotation with the value coming from live link. */
	UPROPERTY(EditAnywhere, Category = "Tracking|nDisplay|LiveLink")
	bool bUseRotation = true;
	
	/** Whether we should set the owning actor's scale with the value coming from live link. */
	UPROPERTY(EditAnywhere, Category = "Tracking|nDisplay|LiveLink")
	bool bUseScale = true;

	/**
	 * Whether we sweep to the destination location, triggering overlaps along the way and stopping short of the target if blocked by something.
	 * Only the root component is swept and checked for blocking collision, child components move without sweeping. If collision is off, this has no effect.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking|nDisplay|LiveLink")
	bool bSweep = false;

	/**
	 * Whether we teleport the physics state (if physics collision is enabled for this object).
	 * If true, physics velocity for this object is unchanged (so ragdoll parts are not affected by change in location).
	 * If false, physics velocity is updated based on the change in position (affecting ragdoll parts).
	 * If CCD is on and not teleporting, this will affect objects along the entire sweep volume.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking|nDisplay|LiveLink")
	bool bTeleport = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking|HMD", BlueprintSetter=SetShowDeviceModel)
	bool bShowDeviceModelInHMD = true;

	UFUNCTION(BlueprintSetter)
	void SetShowDeviceModel(const bool bShowControllerModel);

	//~ Begin SceneComponent Interface	
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End SceneComponent Interface

private:

	/**
	 * @brief Helper function that applies the LiveLink data to this component. Taken from the LiveLink Transform Controller.
	 * @param Transform Transform read from the LiveLink Client.
	 * @param StaticData Static data from the LiveLink Subject, defines what is and isn't supported.
	 * @return void
	 */
	void ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData);
	
	USceneComponent* TrackedComponent = nullptr;
	UMotionControllerComponent* GetMotionControllerComponentByMotionSource(EControllerHand MotionSource) const;	
	
};
