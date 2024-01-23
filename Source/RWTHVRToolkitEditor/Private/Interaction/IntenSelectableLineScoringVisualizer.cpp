#include "Interaction/IntenSelectableLineScoringVisualizer.h"

#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableLineScoring.h"

IMPLEMENT_HIT_PROXY(FLinePointProxy, HComponentVisProxy);

FIntenSelectableLineScoringVisualizer::FIntenSelectableLineScoringVisualizer()
{
	
}

FIntenSelectableLineScoringVisualizer::~FIntenSelectableLineScoringVisualizer()
{
	
}

FVector FIntenSelectableLineScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentLineSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}
	return LineBehavior->GetComponentTransform().TransformPosition(LineBehavior->LinePoints[CurrentLineSelectionIndex]);
}

bool FIntenSelectableLineScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FLinePointProxy::StaticGetType()))
		{
			const UIntenSelectableLineScoring* T = Cast<const UIntenSelectableLineScoring>(VisProxy->Component.Get());
			LineBehavior = const_cast<UIntenSelectableLineScoring*>(T);
			FLinePointProxy* Proxy = (FLinePointProxy*)VisProxy;
			CurrentLineSelectionIndex = Proxy->TargetIndex;
		}else
		{
			CurrentLineSelectionIndex = INDEX_NONE;
		}
	}
	else
	{
		CurrentLineSelectionIndex = INDEX_NONE;
	}

	return bEditing;
}

void FIntenSelectableLineScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableLineScoring* ComponentCasted = Cast<UIntenSelectableLineScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		for(int i = 0; i < ComponentCasted->LinePoints.Num(); i++)
		{
			PDI->SetHitProxy(new FLinePointProxy(Component, i));
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}
		const FVector Start = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[0]);
		const FVector End = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[1]);
		PDI->DrawLine(Start, End, FColor::Green, SDPG_World);
	}
}

void FIntenSelectableLineScoringVisualizer::EndEditing()
{
	LineBehavior->MarkRenderStateDirty();
	GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableLineScoringVisualizer::GetEditedComponent() const
{
	return LineBehavior;
}

bool FIntenSelectableLineScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentLineSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());
		
		const FVector WorldSelection = LineBehavior->GetComponentTransform().TransformPosition(LineBehavior->LinePoints[CurrentLineSelectionIndex]);
		const FVector NewWorldPos = LineBehavior->GetComponentTransform().InverseTransformPosition(WorldSelection + DeltaTranslate);
		LineBehavior->LinePoints[CurrentLineSelectionIndex] = NewWorldPos;
		
		LineBehavior->MarkRenderStateDirty();
		GEditor->RedrawLevelEditingViewports(true);
		bHandled = true;
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableLineScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (CurrentLineSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


