// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "UniversalTrackedComponent.generated.h"

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
class DISPLAYCLUSTEREXTENSIONS_API UUniversalTrackedComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUniversalTrackedComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking") ETrackedComponentType ProxyType = ETrackedComponentType::TCT_HEAD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking") EAttachementType AttachementType = EAttachementType::AT_FLYSTICK;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	USceneComponent* TrackedComponent = nullptr;
	UMotionControllerComponent* GetMotionControllerComponentByMotionSource(EControllerHand MotionSource);
	USceneComponent* GetComponentForSelectedAttachment(EAttachementType AttachmentType) const;
};
