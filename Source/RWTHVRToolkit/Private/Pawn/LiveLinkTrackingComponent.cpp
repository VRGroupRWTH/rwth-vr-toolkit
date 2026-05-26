// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LiveLinkTrackingComponent.h"

#include "ILiveLinkClient.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "Utility/RWTHVRUtilities.h"

ULiveLinkTrackingComponent::ULiveLinkTrackingComponent() { PrimaryComponentTick.bCanEverTick = true; }

void ULiveLinkTrackingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
											   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TrackedComponent)
		EvaluateLivelink();
}

void ULiveLinkTrackingComponent::EvaluateLivelink() const
{
	if (URWTHVRUtilities::IsRoomMountedMode() && GetOwner()->HasLocalNetOwner())
	{
		if (bDisableLiveLink || SubjectRepresentation.Subject.IsNone() || SubjectRepresentation.Role == nullptr)
		{
			return;
		}

		// Get the LiveLink interface and evaluate the current existing frame data for the given Subject and Role.
		ILiveLinkClient& LiveLinkClient =
			IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		FLiveLinkSubjectFrameData SubjectData;
		const bool bHasValidData = LiveLinkClient.EvaluateFrame_AnyThread(SubjectRepresentation.Subject,
																		  SubjectRepresentation.Role, SubjectData);

		if (!bHasValidData)
		{
			return;
		}

		// Assume we are using a Transform Role to track the components! This is a slightly dangerous assumption, and
		// could be further improved.
		const FLiveLinkTransformStaticData* StaticData = SubjectData.StaticData.Cast<FLiveLinkTransformStaticData>();
		const FLiveLinkTransformFrameData* FrameData = SubjectData.FrameData.Cast<FLiveLinkTransformFrameData>();

		if (StaticData && FrameData)
		{
			// Finally, apply the transform to this component according to the static data.
			ApplyLiveLinkTransform(FrameData->Transform, *StaticData);
		}
	}
}

void ULiveLinkTrackingComponent::ApplyLiveLinkTransform(const FTransform& Transform,
														const FLiveLinkTransformStaticData& StaticData) const
{
	if (StaticData.bIsLocationSupported)
	{
		if (bWorldTransform)
		{
			TrackedComponent->SetWorldLocation(Transform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
		else
		{
			TrackedComponent->SetRelativeLocation(Transform.GetLocation(), false, nullptr,
												  ETeleportType::TeleportPhysics);
		}
	}

	if (StaticData.bIsRotationSupported)
	{
		if (bWorldTransform)
		{
			TrackedComponent->SetWorldRotation(Transform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
		else
		{
			TrackedComponent->SetRelativeRotation(Transform.GetRotation(), false, nullptr,
												  ETeleportType::TeleportPhysics);
		}
	}

	if (StaticData.bIsScaleSupported)
	{
		if (bWorldTransform)
		{
			TrackedComponent->SetWorldScale3D(Transform.GetScale3D());
		}
		else
		{
			TrackedComponent->SetRelativeScale3D(Transform.GetScale3D());
		}
	}
}
