// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/GrabbingBehaviorPlaneVisualizer.h"
#include "Interaction/GrabbingBehaviorOnPlaneComponent.h"
#include "SceneManagement.h"

FGrabbingBehaviorPlaneVisualizer::FGrabbingBehaviorPlaneVisualizer()
{
}

FGrabbingBehaviorPlaneVisualizer::~FGrabbingBehaviorPlaneVisualizer()
{
}

void FGrabbingBehaviorPlaneVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
                                                         FPrimitiveDrawInterface* PDI)
{
	const UGrabbingBehaviorOnPlaneComponent* PlaneBehavior = Cast<const UGrabbingBehaviorOnPlaneComponent>(Component);

	if (PlaneBehavior != nullptr)
	{
		FVector Attachment = PlaneBehavior->GetComponentLocation();
		FVector Forward = PlaneBehavior->GetComponentQuat().GetUpVector();
		FVector Right = PlaneBehavior->GetComponentQuat().GetRightVector();
		FVector Next;
		Right.Normalize();


		float Distance = PlaneBehavior->GetDistance();
		int Segments = 60;
		check(360% Segments == 0 && "circle cannot be divided equally");

		for (int i = 1; i < Segments + 1; i++) // draw circle using lines
		{
			Next = Right.RotateAngleAxis(360 / Segments, Forward);

			PDI->DrawLine(Attachment + Right * Distance, Attachment + Next * Distance, FColor::Blue, SDPG_World);
			Right = Next;
		}
	}
}
