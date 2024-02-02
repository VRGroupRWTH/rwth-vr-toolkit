#include "Interaction/IntenSelectableMultiPointScoringVisualizer.h"

#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableMultiPointScoring.h"

IMPLEMENT_HIT_PROXY(FMultiPointProxy, HComponentVisProxy);

FIntenSelectableMultiPointScoringVisualizer::FIntenSelectableMultiPointScoringVisualizer()
{
	
}

FIntenSelectableMultiPointScoringVisualizer::~FIntenSelectableMultiPointScoringVisualizer()
{
}

FVector FIntenSelectableMultiPointScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}
	return MultiPointBehaviour->GetComponentTransform().TransformPosition(MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex]);
}

bool FIntenSelectableMultiPointScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableMultiPointScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableMultiPointScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                      HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FMultiPointProxy::StaticGetType()))
		{
			const UIntenSelectableMultiPointScoring* T = Cast<const UIntenSelectableMultiPointScoring>(VisProxy->Component.Get());
			MultiPointBehaviour = const_cast<UIntenSelectableMultiPointScoring*>(T);
			FMultiPointProxy* Proxy = (FMultiPointProxy*)VisProxy;
			CurrentSelectionIndex = Proxy->TargetIndex;
		}else
		{
			CurrentSelectionIndex = INDEX_NONE;
		}
	}
	else
	{
		CurrentSelectionIndex = INDEX_NONE;
	}

	return bEditing;
}


// Fill out your copyright notice in the Description page of Project Settings.

void FIntenSelectableMultiPointScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableMultiPointScoring* ComponentCasted = Cast<UIntenSelectableMultiPointScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		for(int i = 0; i < ComponentCasted->PointsToSelect.Num(); i++)
		{
			PDI->SetHitProxy(new FMultiPointProxy(Component, i));
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->PointsToSelect[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}
	}
}

void FIntenSelectableMultiPointScoringVisualizer::EndEditing()
{
	MultiPointBehaviour->MarkRenderStateDirty();
	GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableMultiPointScoringVisualizer::GetEditedComponent() const
{
	return MultiPointBehaviour;
}

bool FIntenSelectableMultiPointScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());
		
		const FVector WorldSelection = MultiPointBehaviour->GetComponentTransform().TransformPosition(MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex]);
		const FVector NewWorldPos = MultiPointBehaviour->GetComponentTransform().InverseTransformPosition(WorldSelection + DeltaTranslate);
		MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex] = NewWorldPos;
		
		MultiPointBehaviour->MarkRenderStateDirty();
		GEditor->RedrawLevelEditingViewports(true);
		bHandled = true;
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableMultiPointScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


