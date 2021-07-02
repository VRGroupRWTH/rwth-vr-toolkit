// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
/**
 * 
 */
class FPrimitiveDrawInterface;
class FSceneView;

class RWTHVRTOOLKITEDITOR_API FGrabbingBehaviorOnLineVisualizer : public FComponentVisualizer
{
public:
	FGrabbingBehaviorOnLineVisualizer();
	~FGrabbingBehaviorOnLineVisualizer();

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};




