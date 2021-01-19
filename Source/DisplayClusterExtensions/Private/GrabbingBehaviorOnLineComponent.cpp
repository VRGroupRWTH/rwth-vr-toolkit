// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabbingBehaviorOnLineComponent.h"

// Sets default values for this component's properties
UGrabbingBehaviorOnLineComponent::UGrabbingBehaviorOnLineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetAbsolute(true, true, true);
	
	this->Distance = 0;
}


void UGrabbingBehaviorOnLineComponent::SetDistance(float Dist)
{

	check(Dist > 0 && "max distance has to be greater than 0");
	this->Distance = Dist;
}

float UGrabbingBehaviorOnLineComponent::GetDistance() const 
{
	return this->Distance;
}

void UGrabbingBehaviorOnLineComponent::HandleNewPositionAndDirection(FVector Position, FQuat Orientation)
{
	FVector AttachmentPoint = this->GetRelativeLocation();
	FVector ConstraintAxis = this->GetComponentQuat().GetUpVector();
	FVector Direction = Orientation.GetForwardVector();
	FVector FromHandToMe = -Position + AttachmentPoint;

	// Vector perpendicular to both points
	FVector Temp = FVector::CrossProduct(FromHandToMe, ConstraintAxis);
	Temp.Normalize();

	FVector PlaneNormal = FVector::CrossProduct(ConstraintAxis,Temp);

	// get intersection point defined by plane
	FVector Intersection =  FMath::LinePlaneIntersection(Position, Position + Direction, AttachmentPoint, PlaneNormal);
	FVector FromOriginToIntersection = Intersection - AttachmentPoint;

	// point along the constraint axis with length of the projection from intersection point onto the axis
	FVector NewPosition = FVector::DotProduct(FromOriginToIntersection, ConstraintAxis)* ConstraintAxis;
	
	NewPosition = NewPosition.GetClampedToMaxSize(Distance);
	NewPosition += AttachmentPoint;

	// transform the targeted actor which is owner of this component with calculated quaternion and posiition
	// here rotation is not changed
	GetOwner()->SetActorLocation(NewPosition);
}

// Called when the game starts
void UGrabbingBehaviorOnLineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGrabbingBehaviorOnLineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

