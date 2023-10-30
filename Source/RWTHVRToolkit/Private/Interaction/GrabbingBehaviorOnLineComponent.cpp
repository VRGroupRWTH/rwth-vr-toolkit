// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/GrabbingBehaviorOnLineComponent.h"

// Sets default values for this component's properties
UGrabbingBehaviorOnLineComponent::UGrabbingBehaviorOnLineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Distance = 0;
}

void UGrabbingBehaviorOnLineComponent::SetDistance(float Dist)
{
	check(Dist > 0 && "max distance has to be greater than 0");
	Distance = Dist;
}

float UGrabbingBehaviorOnLineComponent::GetDistance() const
{
	return Distance;
}

void UGrabbingBehaviorOnLineComponent::SetDiscreteNumberOfPoints(int Num)
{
	NumPoints = Num;
	bIsDiscrete = true;
}

void UGrabbingBehaviorOnLineComponent::HandleGrabHold(FVector Position, FQuat Orientation)
{
	const FVector AttachmentPoint = this->GetRelativeLocation();
	const FVector ConstraintAxis = this->GetComponentQuat().GetUpVector();
	const FVector Direction = Orientation.GetForwardVector();
	const FVector FromHandToMe = -Position + AttachmentPoint;

	// Vector perpendicular to both points
	FVector Temp = FVector::CrossProduct(FromHandToMe, ConstraintAxis);
	Temp.Normalize();

	const FVector PlaneNormal = FVector::CrossProduct(ConstraintAxis, Temp);

	// get intersection point defined by plane
	const FVector Intersection = FMath::LinePlaneIntersection(Position, Position + Direction, AttachmentPoint,
	                                                          PlaneNormal);
	const FVector FromOriginToIntersection = Intersection - AttachmentPoint;

	// point along the constraint axis with length of the projection from intersection point onto the axis
	FVector NewPosition = FVector::DotProduct(FromOriginToIntersection, ConstraintAxis) * ConstraintAxis;

	NewPosition = NewPosition.GetClampedToMaxSize(Distance);

	if (bIsDiscrete)
	{
		const float lengthOfSegment = 1.f / static_cast<float>(NumPoints + 1.f);
		const FVector LineBeginning = -ConstraintAxis * Distance;
		const float LengthOnLine = (FVector::DotProduct(FromOriginToIntersection, ConstraintAxis) / Distance + 1.f) / 2.f;
		// is between 0 and 1

		float VectorSize = FMath::CeilToFloat(LengthOnLine / lengthOfSegment);
		if (VectorSize <= 0) VectorSize = 1;
		if (VectorSize > NumPoints) VectorSize = NumPoints;
		NewPosition = LineBeginning + VectorSize * ConstraintAxis * lengthOfSegment * Distance * 2.f;
	}

	NewPosition += AttachmentPoint;

	// transform the targeted actor which is owner of this component with calculated quaternion and posiition
	// here rotation is not changed
	GetOwner()->SetActorLocation(NewPosition);
}
