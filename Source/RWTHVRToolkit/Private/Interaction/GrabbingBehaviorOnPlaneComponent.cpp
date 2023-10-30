// Fill out your copyright notice in the Description page of Project Settings.
//TODO rename distance to maxDistance

#include "Interaction/GrabbingBehaviorOnPlaneComponent.h"

void UGrabbingBehaviorOnPlaneComponent::SetDistance(float Dist)
{
	check(Dist > 0 && "max distance has to be greater than 0");
	this->Distance = Dist;
}

float UGrabbingBehaviorOnPlaneComponent::GetDistance() const
{
	return this->Distance;
}

void UGrabbingBehaviorOnPlaneComponent::HandleGrabHold(FVector Position, FQuat Orientation)
{
	FVector AttachmentPoint = this->GetRelativeLocation();
	FVector PlaneNormal = this->GetComponentQuat().GetUpVector();
	FVector Direction = Orientation.GetForwardVector();

	// calculate point on plane which is pointed to by hand ray
	FVector Intersection = FMath::LinePlaneIntersection(Position, Position + Direction, AttachmentPoint, PlaneNormal);
	FVector NewPosition = -AttachmentPoint + Intersection;

	// clamp size by maxDistance
	NewPosition = NewPosition.GetClampedToMaxSize(Distance);

	// after this NewPoint is in world position
	NewPosition += AttachmentPoint;

	// set new position and orientation using calculated quaternion and position
	// here rotation is not changed
	GetOwner()->SetActorLocation(NewPosition);
}