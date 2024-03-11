#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Materials/MaterialRenderProxy.h"

class UIntenSelectableCubeScoring;
class FPrimitiveDrawInterface;
class FSceneView;

struct FCubeProxy : HComponentVisProxy 
{
	DECLARE_HIT_PROXY();

	FCubeProxy (const UActorComponent* InComponent, int32 InTargetIndex)
	: HComponentVisProxy (InComponent)
	, TargetIndex(InTargetIndex)
	{}

	int32 TargetIndex;
};


class RWTHVRTOOLKITEDITOR_API FIntenSelectableCubeScoringVisualizer : public FComponentVisualizer
{
private:
	int CurrentSelectionIndex;

	FColoredMaterialRenderProxy DebugMaterial;
	FProperty* PointsProperty;
	FComponentPropertyPath ScoringBehaviourPropertyPath;
	
public:
	FIntenSelectableCubeScoringVisualizer();
	~FIntenSelectableCubeScoringVisualizer();

	FVector GetCurrentVectorWorld() const;

	virtual bool IsVisualizingArchetype() const override;
	UIntenSelectableCubeScoring* GetEditedScoringComponent() const;

	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;
	virtual UActorComponent* GetEditedComponent() const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
};
