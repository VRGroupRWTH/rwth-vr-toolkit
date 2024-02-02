// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
class UIntenSelectableMultiPointScoring;
/**
 * 
 */
class FPrimitiveDrawInterface;
class FSceneView;

struct FMultiPointProxy : HComponentVisProxy 
{
	DECLARE_HIT_PROXY();

	FMultiPointProxy (const UActorComponent* InComponent, int32 InTargetIndex)
	: HComponentVisProxy (InComponent)
	, TargetIndex(InTargetIndex)
	{}

	int32 TargetIndex;
};


class RWTHVRTOOLKITEDITOR_API FIntenSelectableMultiPointScoringVisualizer : public FComponentVisualizer
{
private:
	int CurrentSelectionIndex;

	UIntenSelectableMultiPointScoring* MultiPointBehaviour;
	
public:
	FIntenSelectableMultiPointScoringVisualizer();
	~FIntenSelectableMultiPointScoringVisualizer();

	FVector GetCurrentVectorWorld() const;

	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;
	virtual UActorComponent* GetEditedComponent() const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
};
