#include "Interaction/IntenSelectableCylinderScoringVisualizer.h"

#include "SceneManagement.h"

IMPLEMENT_HIT_PROXY(FCylinderPointProxy, HComponentVisProxy);

FIntenSelectableCylinderScoringVisualizer::FIntenSelectableCylinderScoringVisualizer() : DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{

}

FIntenSelectableCylinderScoringVisualizer::~FIntenSelectableCylinderScoringVisualizer()
{
	
}

FVector FIntenSelectableCylinderScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentCylinderSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}
	return CylinderBehavior->GetComponentTransform().TransformPosition(CylinderBehavior->LinePoints[CurrentCylinderSelectionIndex]);
}

bool FIntenSelectableCylinderScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableCylinderScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableCylinderScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                    HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FCylinderPointProxy::StaticGetType()))
		{
			const UIntenSelectableCylinderScoring* T = Cast<const UIntenSelectableCylinderScoring>(VisProxy->Component.Get());
			CylinderBehavior = const_cast<UIntenSelectableCylinderScoring*>(T);
			FCylinderPointProxy* Proxy = (FCylinderPointProxy*)VisProxy;
			CurrentCylinderSelectionIndex = Proxy->TargetIndex;
		}else
		{
			CurrentCylinderSelectionIndex = INDEX_NONE;
		}
	}
	else
	{
		CurrentCylinderSelectionIndex = INDEX_NONE;
	}

	return bEditing;
}

void FIntenSelectableCylinderScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableCylinderScoring* ComponentCasted = Cast<UIntenSelectableCylinderScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		for(int i = 0; i < ComponentCasted->LinePoints.Num(); i++)
		{
			PDI->SetHitProxy(new FCylinderPointProxy(Component, i));
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}
		const FVector Start = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[0]);
		const FVector End = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[1]);
		PDI->DrawLine(Start, End, FColor::Green, SDPG_World);

		const float Dist = (End-Start).Size();
		DrawCylinder(PDI, Start, End, ComponentCasted->Radius,20 , &DebugMaterial, 0);
	}
}

void FIntenSelectableCylinderScoringVisualizer::EndEditing()
{
	CurrentCylinderSelectionIndex = INDEX_NONE;
	//CylinderBehavior->MarkRenderStateDirty();
	//GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableCylinderScoringVisualizer::GetEditedComponent() const
{
	return CylinderBehavior;
}

bool FIntenSelectableCylinderScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentCylinderSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Delta: %s"), *DeltaTranslate.ToString());
		UE_LOG(LogTemp, Warning, TEXT("Current Index: %d"), CurrentCylinderSelectionIndex);
		
		const FVector WorldSelection = CylinderBehavior->GetComponentTransform().TransformPosition(CylinderBehavior->LinePoints[CurrentCylinderSelectionIndex]);
		const FVector NewWorldPos = CylinderBehavior->GetComponentTransform().InverseTransformPosition(WorldSelection + DeltaTranslate);
		//CylinderBehavior->LinePoints[CurrentCylinderSelectionIndex] = NewWorldPos;
		CylinderBehavior->LinePoints[CurrentCylinderSelectionIndex] += DeltaTranslate;
		LinePoints[CurrentCylinderSelectionIndex] += DeltaTranslate;

		UE_LOG(LogTemp, Warning, TEXT("Component: %s"), *(CylinderBehavior->LinePoints[CurrentCylinderSelectionIndex]).ToString());
		UE_LOG(LogTemp, Warning, TEXT("self: %s"), *(LinePoints[CurrentCylinderSelectionIndex]).ToString());
				
		//CylinderBehavior->MarkRenderStateDirty();
		//GEditor->RedrawLevelEditingViewports(true);

		const FVector Average = (LinePoints[0] + LinePoints[1])/ 2;
		const FVector ShiftToMiddle = Average;

		CylinderBehavior->SetWorldLocation(CylinderBehavior->GetComponentTransform().TransformPositionNoScale(Average));
		CylinderBehavior->LinePoints[0] -= ShiftToMiddle;
		CylinderBehavior->LinePoints[1] -= ShiftToMiddle;
		
		bHandled = true;
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableCylinderScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (GetEditedComponent() && CurrentCylinderSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


