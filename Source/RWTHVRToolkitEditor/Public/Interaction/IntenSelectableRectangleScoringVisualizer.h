// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
class UIntenSelectableRectangleScoring;
/**
 * 
 */
class FPrimitiveDrawInterface;
class FSceneView;

struct FRectangleProxy : HComponentVisProxy 
{
	DECLARE_HIT_PROXY();

	FRectangleProxy (const UActorComponent* InComponent, int32 InTargetIndex)
	: HComponentVisProxy (InComponent)
	, TargetIndex(InTargetIndex)
	{}

	int32 TargetIndex;
};


class RWTHVRTOOLKITEDITOR_API FIntenSelectableRectangleScoringVisualizer : public FComponentVisualizer
{
private:
	int CurrentSelectionIndex;

	UIntenSelectableRectangleScoring* RectangleBehaviour;
	
public:
	FIntenSelectableRectangleScoringVisualizer();
	~FIntenSelectableRectangleScoringVisualizer();

	FVector GetCurrentVectorWorld() const;

	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;
	virtual UActorComponent* GetEditedComponent() const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
};
