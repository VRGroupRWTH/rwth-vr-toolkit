// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "IntenSelectableMultiPointScoringVisualizer.generated.h"

class UIntenSelectableMultiPointScoring;
class FPrimitiveDrawInterface;
class FSceneView;

/** Selection state data that will be captured by scoped transactions.*/
UCLASS()
class UMyComponentVisualizerSelectionState : public UObject
{
	GENERATED_BODY()
public:
	void Reset();
	void SetSplinePropertyPath(const FComponentPropertyPath& InSplinePropertyPath) { SplinePropertyPath = InSplinePropertyPath; }
	const FComponentPropertyPath GetSplinePropertyPath() const { return SplinePropertyPath; }
protected:
	/** Property path from the parent actor to the component */
	UPROPERTY()
	FComponentPropertyPath SplinePropertyPath;

	/** Position on spline we have selected */
	UPROPERTY()
	FVector SelectedSplinePosition;
};


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

	TObjectPtr<UMyComponentVisualizerSelectionState> SelectionState;

	UIntenSelectableMultiPointScoring* MultiPointBehaviour;

	FComponentPropertyPath SplinePropertyPath;
	
public:
	FIntenSelectableMultiPointScoringVisualizer();
	~FIntenSelectableMultiPointScoringVisualizer();

	FVector GetCurrentVectorWorld() const;
	
	virtual bool IsVisualizingArchetype() const override;
	virtual bool ShowWhenSelected() override;
	virtual bool ShouldShowForSelectedSubcomponents(const UActorComponent* Component) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;
	virtual UActorComponent* GetEditedComponent() const override;

	UIntenSelectableMultiPointScoring* GetEdited() const;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

	static void MyNotifyPropertiesModified(USceneComponent* Component, const TArray<FProperty*>& Properties, EPropertyChangeType::Type PropertyChangeType = EPropertyChangeType::Unspecified);

	UIntenSelectableMultiPointScoring* GetEditedScoringComponent() const;
};
