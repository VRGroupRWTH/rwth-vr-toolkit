#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class UIntenSelectableMultiPointScoring;
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
	FProperty* PointsProperty;
	FComponentPropertyPath ScoringBehaviourPropertyPath;
	
public:
	FIntenSelectableMultiPointScoringVisualizer();
	~FIntenSelectableMultiPointScoringVisualizer();
	
	virtual bool IsVisualizingArchetype() const override;
	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;
	
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;

	FVector GetCurrentVectorWorld() const;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
	virtual UActorComponent* GetEditedComponent() const override;
	UIntenSelectableMultiPointScoring* GetEditedScoringComponent() const;
	
	virtual void EndEditing() override;
};
