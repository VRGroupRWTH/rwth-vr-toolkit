// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/UniversalTrackedComponent.h"

#include "Camera/CameraComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "ILiveLinkClient.h"

DEFINE_LOG_CATEGORY(LogUniversalTrackedComponent);

// Sets default values for this component's properties
UUniversalTrackedComponent::UUniversalTrackedComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
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
		if (ProxyType == ETrackedComponentType::TCT_HEAD)
		{
			TrackedComponent = GetOwner()->FindComponentByClass<UCameraComponent>();
		}
		else
		{
			/* Spawn Motion Controller Components in HMD Mode*/
			UMotionControllerComponent* MotionController = Cast<UMotionControllerComponent>(GetOwner()->AddComponentByClass(UMotionControllerComponent::StaticClass(), false, FTransform::Identity, false));

			// Todo: If bAlwaysUseLiveLinkTracking is true, those should be sourced by LiveLink
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
		if (TrackedComponent)
			AttachToComponent(Cast<USceneComponent>(TrackedComponent), FAttachmentTransformRules::SnapToTargetIncludingScale);

	}
	// Check for bAlwaysUseLiveLinkTracking here too in case we want to use LiveLink in Editor/Desktop mode.
	else if (UVirtualRealityUtilities::IsDesktopMode() && !bAlwaysUseLiveLinkTracking)
	{
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
		if (TrackedComponent) AttachToComponent(Cast<USceneComponent>(TrackedComponent), FAttachmentTransformRules::SnapToTargetIncludingScale);

	}
	// If we're either in the cave or using LiveLink, set it up here. Might want to differentiate between the two cases later on,
	// for now both should be equivalent. Maybe set up some additional stuff automatically like presets etc.
	else if (UVirtualRealityUtilities::IsRoomMountedMode() || bAlwaysUseLiveLinkTracking)
	{
		// Instead of using the clumsy LiveLinkComponentController, we just directly check the LiveLink Data in Tick later on.
		// Set up this Component to Tick, and check whether Subject and Role is set.

		// TODO: Check for AttachementType and automatically get the respective Subject/Role. Need to investigate how those are called by DTrack.
		TrackedComponent = this;
		bUseLiveLinkTracking = true; // override this in case someone forgot to set it.
		if (SubjectRepresentation.Subject.IsNone() || SubjectRepresentation.Role == nullptr)
		{
			UE_LOG(LogUniversalTrackedComponent, Error, TEXT("UUniversalTrackedComponent::BeginPlay(): No LiveLink Subject or Role is set! Tracking will not work"));
		}
		SetComponentTickEnabled(true);
	}
}

void UUniversalTrackedComponent::PostLoad()
{
	Super::PostLoad();

	// This is required in PostLoad (and theoretically in OnComponentCreated too) because just setting this in
	// the constructor or PostInitializeProperties will load the CDO's property values.
	// Just calling it in BeginPlay() won't let us see the LiveLink preview in the editor. 
	bTickInEditor = bAlwaysUseLiveLinkTracking;	
	PrimaryComponentTick.bStartWithTickEnabled =  bAlwaysUseLiveLinkTracking;	
}

#if WITH_EDITOR
void UUniversalTrackedComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Catch the change to bAlwaysUseLiveLinkTracking, and disable/enable tick respectively for in-editor tracking.
	const FName PropertyName(PropertyChangedEvent.GetPropertyName());
	if(PropertyName == GET_MEMBER_NAME_CHECKED(UUniversalTrackedComponent, bAlwaysUseLiveLinkTracking))
	{
		bTickInEditor = bAlwaysUseLiveLinkTracking;
		SetComponentTickEnabled(bAlwaysUseLiveLinkTracking);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


void UUniversalTrackedComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check whether we need to update the component with new LiveLink data. This is either the case if
	// nDisplay uses bUseLiveLinkTracking (Play Mode), or if bAlwaysUseLiveLinkTracking is true, then we tick both in Play Mode and Editor.
	const bool bEvaluateLiveLink = bUseLiveLinkTracking || bAlwaysUseLiveLinkTracking;

	if(!bEvaluateLiveLink || SubjectRepresentation.Subject.IsNone() || SubjectRepresentation.Role == nullptr)
	{
		return;
	}

	// Get the LiveLink interface and evaluate the current existing frame data for the given Subject and Role.
	ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	FLiveLinkSubjectFrameData SubjectData;
	const bool bHasValidData = LiveLinkClient.EvaluateFrame_AnyThread(SubjectRepresentation.Subject, SubjectRepresentation.Role, SubjectData);

	if(!bHasValidData)
		return;

	// Assume we are using a Transform Role to track the components! This is a slightly dangerous assumption, and could be further improved.
	const FLiveLinkTransformStaticData* StaticData = SubjectData.StaticData.Cast<FLiveLinkTransformStaticData>();
	const FLiveLinkTransformFrameData* FrameData = SubjectData.FrameData.Cast<FLiveLinkTransformFrameData>();

	if (StaticData && FrameData)
	{
		// Finally, apply the transform to this component according to the static data.
		ApplyLiveLinkTransform(FrameData->Transform, *StaticData);
	}	
}

UMotionControllerComponent* UUniversalTrackedComponent::GetMotionControllerComponentByMotionSource(EControllerHand MotionSource) const
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

void UUniversalTrackedComponent::ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData)
{
	if (bUseLocation && StaticData.bIsLocationSupported)
	{
		if (bWorldTransform)
		{
			this->SetWorldLocation(Transform.GetLocation(), bSweep, nullptr, bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::ResetPhysics);
		}
		else
		{
			this->SetRelativeLocation(Transform.GetLocation(), bSweep, nullptr, bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::ResetPhysics);
		}
	}

	if (bUseRotation && StaticData.bIsRotationSupported)
	{
		if (bWorldTransform)
		{
			this->SetWorldRotation(Transform.GetRotation(), bSweep, nullptr, bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::ResetPhysics);
		}
		else
		{
			this->SetRelativeRotation(Transform.GetRotation(), bSweep, nullptr, bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::ResetPhysics);
		}
	}

	if (bUseScale && StaticData.bIsScaleSupported)
	{
		if (bWorldTransform)
		{
			this->SetWorldScale3D(Transform.GetScale3D());
		}
		else
		{
			this->SetRelativeScale3D(Transform.GetScale3D());
		}
	}	
}
