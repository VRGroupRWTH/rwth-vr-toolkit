#include "Interaction/IntenSelectableMultiPointScoringVisualizer.h"
#include "ActorEditorUtils.h"
#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableMultiPointScoring.h"

IMPLEMENT_HIT_PROXY(FMultiPointProxy, HComponentVisProxy);

FIntenSelectableMultiPointScoringVisualizer::FIntenSelectableMultiPointScoringVisualizer()
{
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableMultiPointScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableMultiPointScoring, PointsToSelect));
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
	return GetEditedScoringComponent()->GetComponentTransform().TransformPosition(GetEditedScoringComponent()->PointsToSelect[CurrentSelectionIndex]);
}

bool FIntenSelectableMultiPointScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());
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
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FMultiPointProxy::StaticGetType()))
		{
			const UIntenSelectableMultiPointScoring* T = Cast<const UIntenSelectableMultiPointScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);
			
			FMultiPointProxy* Proxy = (FMultiPointProxy*)VisProxy;
			CurrentSelectionIndex = Proxy->TargetIndex;
			//UE_LOG(LogTemp, Warning, TEXT("Handling Click %i"), CurrentSelectionIndex);
		}else
		{
			CurrentSelectionIndex = INDEX_NONE;
			//UE_LOG(LogTemp, Warning, TEXT("Handling Click => no selection"));
		}
	}
	else
	{
		CurrentSelectionIndex = INDEX_NONE;
		//UE_LOG(LogTemp, Warning, TEXT("Handling Click => no selection"));

	}

	return bEditing;
}

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

}

UActorComponent* FIntenSelectableMultiPointScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

bool FIntenSelectableMultiPointScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UIntenSelectableMultiPointScoring* ScoringComponent = GetEditedScoringComponent();
		ScoringComponent->Modify();
		
		const FVector WorldSelection = ScoringComponent->GetComponentTransform().TransformPosition(GetEditedScoringComponent()->PointsToSelect[CurrentSelectionIndex]);
		const FVector NewWorldPos = ScoringComponent->GetComponentTransform().InverseTransformPosition(WorldSelection + DeltaTranslate);
		ScoringComponent->PointsToSelect[CurrentSelectionIndex] = NewWorldPos;

		//UE_LOG(LogTemp, Warning, TEXT("New Pos: %s"), *ScoringComponent->PointsToSelect[CurrentSelectionIndex].ToString());
		
		ScoringComponent->MarkRenderStateDirty();
		bHandled = true;
		
		TArray<FProperty*> Properties;
		Properties.Add(PointsProperty);
		NotifyPropertiesModified(ScoringComponent, Properties, EPropertyChangeType::ValueSet);
		
		GEditor->RedrawLevelEditingViewports(false);

	}else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

UIntenSelectableMultiPointScoring* FIntenSelectableMultiPointScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableMultiPointScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableMultiPointScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
                                                                    FVector& OutLocation) const
{
	if (GetEditedScoringComponent() && CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}
