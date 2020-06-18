// Fill out your copyright notice in the Description page of Project Settings.
//TODO rename distance to maxDistance

#include "GrabbingBehaviorOnPlaneComponent.h"

// Sets default values for this component's properties
UGrabbingBehaviorOnPlaneComponent::UGrabbingBehaviorOnPlaneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bAbsoluteLocation = true;
	bAbsoluteRotation = true;
	bAbsoluteScale    = true;
	// ...
}


void UGrabbingBehaviorOnPlaneComponent::SetDistance(float Dist)
{
	check(Dist > 0 && "max distance has to be greater than 0");
	this->Distance = Dist;
}

float UGrabbingBehaviorOnPlaneComponent::GetDistance() const 
{
	return this->Distance;
}


void UGrabbingBehaviorOnPlaneComponent::HandleNewPositionAndDirection(FVector Position, FQuat Orientation)
{
	FVector AttachmentPoint = this->RelativeLocation;
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


// Called when the game starts
void UGrabbingBehaviorOnPlaneComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGrabbingBehaviorOnPlaneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...

}

