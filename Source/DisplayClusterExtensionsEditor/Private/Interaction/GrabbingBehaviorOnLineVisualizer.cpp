// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/GrabbingBehaviorOnLineVisualizer.h"
#include "Interaction/GrabbingBehaviorOnLineComponent.h"

#include "SceneManagement.h"

FGrabbingBehaviorOnLineVisualizer::FGrabbingBehaviorOnLineVisualizer()
{
}

FGrabbingBehaviorOnLineVisualizer::~FGrabbingBehaviorOnLineVisualizer()
{
}


// Fill out your copyright notice in the Description page of Project Settings.

void FGrabbingBehaviorOnLineVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {


	const UGrabbingBehaviorOnLineComponent* LineBehavior = Cast<const UGrabbingBehaviorOnLineComponent>(Component);

	if (LineBehavior != nullptr)
	{
		FVector Attachment = LineBehavior->GetComponentLocation();
		FVector Forward = LineBehavior->GetComponentQuat().GetUpVector();
		float Distance = LineBehavior->GetDistance();

		PDI->DrawLine(Attachment + Forward * Distance, Attachment - Forward * Distance, FColor::Blue, SDPG_World);
	}
}


