#include "Interaction/IntenSelectableLineScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableLineScoring.h"

IMPLEMENT_HIT_PROXY(FLinePointProxy, HComponentVisProxy);

FIntenSelectableLineScoringVisualizer::FIntenSelectableLineScoringVisualizer()
{
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableLineScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableLineScoring, LinePoints));
}

FIntenSelectableLineScoringVisualizer::~FIntenSelectableLineScoringVisualizer()
{
	
}

FVector FIntenSelectableLineScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentLineSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}else if(CurrentLineSelectionIndex == 2)
	{
		return GetEditedScoringComponent()->GetComponentLocation();
	}
	return GetEditedScoringComponent()->GetComponentTransform().TransformPositionNoScale(GetEditedScoringComponent()->LinePoints[CurrentLineSelectionIndex]);
}

bool FIntenSelectableLineScoringVisualizer::IsVisualizingArchetype() const
{
	return (GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner()));
}

bool FIntenSelectableLineScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableLineScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableLineScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	//UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FLinePointProxy::StaticGetType()))
		{
			const UIntenSelectableLineScoring* T = Cast<const UIntenSelectableLineScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);
			
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
	
	if (ComponentCasted != nullptr && ComponentCasted->LinePoints.Num() == 2)
	{
		for(int i = 0; i < ComponentCasted->LinePoints.Num() && i <= 2; i++)
		{
			PDI->SetHitProxy(new FLinePointProxy(Component, i));
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}

		PDI->SetHitProxy(new FLinePointProxy(Component, 2));
		
		const FVector Start = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[0]);
		const FVector End = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->LinePoints[1]);
		PDI->DrawLine(Start, End, FColor::Green, SDPG_Foreground, 3);

		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableLineScoringVisualizer::EndEditing()
{
	GetEditedScoringComponent()->MarkRenderStateDirty();
	GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableLineScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

UIntenSelectableLineScoring* FIntenSelectableLineScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableLineScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableLineScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentLineSelectionIndex != INDEX_NONE)
	{
		UIntenSelectableLineScoring* ScoringComponent = GetEditedScoringComponent();
		
		if(ScoringComponent->LinePoints.Num() == 2)
		{

			if(CurrentLineSelectionIndex == 2)
			{
				ScoringComponent->AddWorldOffset(DeltaTranslate);
				ScoringComponent->AddWorldRotation(DeltaRotate);
			}else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());
		
				const FVector WorldSelection = ScoringComponent->GetComponentTransform().TransformPositionNoScale(GetEditedScoringComponent()->LinePoints[CurrentLineSelectionIndex]);
				const FVector NewWorldPos = ScoringComponent->GetComponentTransform().InverseTransformPositionNoScale(WorldSelection + DeltaTranslate);
				ScoringComponent->LinePoints[CurrentLineSelectionIndex] = NewWorldPos;

				const FVector Average = (ScoringComponent->LinePoints[0] + ScoringComponent->LinePoints[1]) / 2;

				ScoringComponent->SetWorldLocation(ScoringComponent->GetComponentTransform().TransformPositionNoScale(Average));
				ScoringComponent->LinePoints[0] -= Average;
				ScoringComponent->LinePoints[1] -= Average;
			}
			

			TArray<FProperty*> Properties;
			Properties.Add(PointsProperty);
			NotifyPropertiesModified(ScoringComponent, Properties, EPropertyChangeType::ValueSet);
		
			ScoringComponent->MarkRenderStateDirty();
			GEditor->RedrawLevelEditingViewports(true);
			bHandled = true;
		}
		
		
	}else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableLineScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (GetEditedScoringComponent() && CurrentLineSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


