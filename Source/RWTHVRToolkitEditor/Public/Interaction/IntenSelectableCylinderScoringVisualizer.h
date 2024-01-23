// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCylinderScoring.h"
#include "Materials/MaterialRenderProxy.h"
class UIntenSelectableLineScoring;
/**
 * 
 */
class FPrimitiveDrawInterface;
class FSceneView;

struct FCylinderPointProxy : HComponentVisProxy 
{
	DECLARE_HIT_PROXY();

	FCylinderPointProxy (const UActorComponent* InComponent, int32 InTargetIndex)
	: HComponentVisProxy (InComponent)
	, TargetIndex(InTargetIndex)
	{}

	int32 TargetIndex;
};


class RWTHVRTOOLKITEDITOR_API FIntenSelectableCylinderScoringVisualizer : public FComponentVisualizer
{
private:
	int CurrentCylinderSelectionIndex;

	UIntenSelectableCylinderScoring* CylinderBehavior;
	FColoredMaterialRenderProxy DebugMaterial;

public:
	FIntenSelectableCylinderScoringVisualizer();
	~FIntenSelectableCylinderScoringVisualizer();

	FVector GetCurrentVectorWorld() const;

	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;
	virtual UActorComponent* GetEditedComponent() const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
};
