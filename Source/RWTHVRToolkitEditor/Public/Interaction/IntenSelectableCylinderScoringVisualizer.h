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

/**Base class for clickable targeting editing proxies*/
struct HTargetingVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HTargetingVisProxy (const UActorComponent* InComponent)
	: HComponentVisProxy(InComponent, HPP_Wireframe)
	{}
};

/**Proxy for target*/
struct HTargetProxy : public HTargetingVisProxy 
{
	DECLARE_HIT_PROXY();

	HTargetProxy (const UActorComponent* InComponent, int32 InTargetIndex)
	: HTargetingVisProxy (InComponent)
	, TargetIndex(InTargetIndex)
	{}

	int32 TargetIndex;
};

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

	//TArray<FVector> LinePoints{FVector::ZeroVector, FVector::ZeroVector};

public:
	FIntenSelectableCylinderScoringVisualizer();
	~FIntenSelectableCylinderScoringVisualizer();

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
