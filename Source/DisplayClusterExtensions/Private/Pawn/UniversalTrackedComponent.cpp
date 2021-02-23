// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/UniversalTrackedComponent.h"


#include "Camera/CameraComponent.h"
#include "Utility/VirtualRealityUtilities.h"

// Sets default values for this component's properties
UUniversalTrackedComponent::UUniversalTrackedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


void UUniversalTrackedComponent::SetShowDeviceModel(const bool bShowControllerModel)
{
	if (!UVirtualRealityUtilities::IsHeadMountedMode() || TrackedComponent == nullptr) return;

	bShowDeviceModelInHMD = bShowControllerModel;
	Cast<UMotionControllerComponent>(TrackedComponent)->SetShowDeviceModel(bShowDeviceModelInHMD);
}

void UUniversalTrackedComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UVirtualRealityUtilities::IsHeadMountedMode())
	{
		if(ProxyType == ETrackedComponentType::TCT_HEAD)
		{
			TrackedComponent = GetOwner()->FindComponentByClass<UCameraComponent>();
		}else
		{
			/* Spawn Motion Controller Components in HMD Mode*/
			UMotionControllerComponent* MotionController = Cast<UMotionControllerComponent>(GetOwner()->AddComponentByClass(UMotionControllerComponent::StaticClass(), false, FTransform::Identity, false));

			switch(ProxyType)
			{
				case ETrackedComponentType::TCT_TRACKER_1:
					MotionController->SetTrackingMotionSource(FName("Special_1"));
					break;
				case ETrackedComponentType::TCT_TRACKER_2:
					MotionController->SetTrackingMotionSource(FName("Special_2"));
					break;
				case ETrackedComponentType::TCT_RIGHT_HAND:
					MotionController->SetTrackingMotionSource(FName("Right"));
					break;
				case ETrackedComponentType::TCT_LEFT_HAND:
					MotionController->SetTrackingMotionSource(FName("Left"));
					break;
				default: break;
			}
			MotionController->SetShowDeviceModel(bShowDeviceModelInHMD);

			TrackedComponent = MotionController;
		}
	}else if(UVirtualRealityUtilities::IsDesktopMode()){
		switch(ProxyType)
		{
			case ETrackedComponentType::TCT_RIGHT_HAND:
			case ETrackedComponentType::TCT_LEFT_HAND:
			case ETrackedComponentType::TCT_HEAD:
				TrackedComponent = GetOwner()->FindComponentByClass<UCameraComponent>(); //All are attached to camera
				break;
			case ETrackedComponentType::TCT_TRACKER_1:
			case ETrackedComponentType::TCT_TRACKER_2:
				TrackedComponent = GetOwner()->GetRootComponent();
				break;
		}
	}
	
	if(TrackedComponent) AttachToComponent(TrackedComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

void UUniversalTrackedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(TrackedComponent) return; /* Already attached */

	/* Test for presence every frame and add as soon as they are there */
	if (UVirtualRealityUtilities::IsRoomMountedMode())
	{
		switch(ProxyType)
		{
			case ETrackedComponentType::TCT_RIGHT_HAND:
			case ETrackedComponentType::TCT_LEFT_HAND:
				TrackedComponent = GetComponentForSelectedAttachment(AttachementType);
				break;
			case ETrackedComponentType::TCT_HEAD:
				TrackedComponent = UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_SHUTTERGLASSES);
				break;
			case ETrackedComponentType::TCT_TRACKER_1:
			case ETrackedComponentType::TCT_TRACKER_2:
				TrackedComponent = GetOwner()->GetRootComponent();
				break;
		}

		if(TrackedComponent) AttachToComponent(TrackedComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

UMotionControllerComponent* UUniversalTrackedComponent::GetMotionControllerComponentByMotionSource(EControllerHand MotionSource)
{
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(UMotionControllerComponent::StaticClass(),Components);
	return Cast<UMotionControllerComponent>(
		Components.FindByPredicate([&MotionSource](UActorComponent* Current)
		{
			switch(MotionSource)
			{
				case EControllerHand::Right:
					return Cast<UMotionControllerComponent>(Current)->MotionSource == FName("Right");
				case EControllerHand::Left:
					return Cast<UMotionControllerComponent>(Current)->MotionSource == FName("Left");
				default: return true;
			}
		})[0]
	);
}

USceneComponent* UUniversalTrackedComponent::GetComponentForSelectedAttachment(EAttachementType AttachmentType) const
{
	switch (AttachmentType)
	{
	case EAttachementType::AT_NONE: return nullptr;
	case EAttachementType::AT_HANDTARGET:
		if(ProxyType == ETrackedComponentType::TCT_RIGHT_HAND) return UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_CAVE_RHT);
		if(ProxyType == ETrackedComponentType::TCT_LEFT_HAND) return UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_CAVE_LHT);
		break;
	case EAttachementType::AT_FLYSTICK:
		return UVirtualRealityUtilities::GetNamedClusterComponent(ENamedClusterComponent::NCC_FLYSTICK);
		break;
	}
	return nullptr;
}

