#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Materials/MaterialRenderProxy.h"

class UIntenSelectableSphereScoring;
class FPrimitiveDrawInterface;
class FSceneView;

struct FSphereProxy : HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	FSphereProxy(const UActorComponent* InComponent, int32 InTargetIndex) :
		HComponentVisProxy(InComponent), TargetIndex(InTargetIndex)
	{
	}

	int32 TargetIndex;
};


class RWTHVRTOOLKITEDITOR_API FIntenSelectableSphereScoringVisualizer : public FComponentVisualizer
{
private:
	int CurrentSelectionIndex;
	FColoredMaterialRenderProxy DebugMaterial;
	FProperty* PointsProperty;
	FComponentPropertyPath ScoringBehaviourPropertyPath;

public:
	FIntenSelectableSphereScoringVisualizer();
	~FIntenSelectableSphereScoringVisualizer();


	virtual bool IsVisualizingArchetype() const override;
	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;

	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
									 const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
								   FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
								  FRotator& DeltaRotate, FVector& DeltaScale) override;

	virtual UActorComponent* GetEditedComponent() const override;
	UIntenSelectableSphereScoring* GetEditedScoringComponent() const;

	FVector GetCurrentVectorWorld() const;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

	virtual void EndEditing() override;
};
