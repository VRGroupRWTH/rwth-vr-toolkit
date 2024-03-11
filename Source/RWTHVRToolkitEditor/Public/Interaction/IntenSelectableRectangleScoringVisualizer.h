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
	FProperty* XLengthProperty;
	FProperty* YLengthProperty;
	FComponentPropertyPath ScoringBehaviourPropertyPath;
	
public:
	FIntenSelectableRectangleScoringVisualizer();
	~FIntenSelectableRectangleScoringVisualizer();
	
	virtual bool IsVisualizingArchetype() const override;
	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;
	
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;

	FVector GetCurrentVectorWorld() const;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
	virtual UActorComponent* GetEditedComponent() const override;
	UIntenSelectableRectangleScoring* GetEditedScoringComponent() const;
	
	virtual void EndEditing() override;
};
