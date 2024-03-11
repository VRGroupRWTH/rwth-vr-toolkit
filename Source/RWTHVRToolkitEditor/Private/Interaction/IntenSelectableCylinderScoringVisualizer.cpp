#include "Interaction/IntenSelectableCylinderScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "SceneManagement.h"

IMPLEMENT_HIT_PROXY(FCylinderPointProxy, HComponentVisProxy);

FIntenSelectableCylinderScoringVisualizer::FIntenSelectableCylinderScoringVisualizer() : DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{
	RadiusProperty = FindFProperty<FProperty>(UIntenSelectableCylinderScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableCylinderScoring, Radius));
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableCylinderScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableCylinderScoring, LinePoints));
}

FIntenSelectableCylinderScoringVisualizer::~FIntenSelectableCylinderScoringVisualizer()
{
	
}

bool FIntenSelectableCylinderScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());
}

FVector FIntenSelectableCylinderScoringVisualizer::GetCurrentVectorWorld() const
{
	if(GetEditedScoringComponent())
	{
		if(CurrentCylinderSelectionIndex == INDEX_NONE)
		{
			return GetEditedScoringComponent()->GetComponentLocation();
		}
		return GetEditedScoringComponent()->GetComponentTransform().TransformPositionNoScale(GetEditedScoringComponent()->LinePoints[CurrentCylinderSelectionIndex]);
	}

	return FVector::ZeroVector;
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
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);
			
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
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPositionNoScale(ComponentCasted->LinePoints[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}

		PDI->SetHitProxy(new FCylinderPointProxy(Component, INDEX_NONE));
		
		const FVector Start = ComponentCasted->GetComponentTransform().TransformPositionNoScale(ComponentCasted->LinePoints[0]);
		const FVector End = ComponentCasted->GetComponentTransform().TransformPositionNoScale(ComponentCasted->LinePoints[1]);
		PDI->DrawLine(Start, End, FColor::Green, SDPG_World);

		const float Dist = (End-Start).Size();
		DrawCylinder(PDI, Start, End, ComponentCasted->Radius,20 , &DebugMaterial, 0);

		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableCylinderScoringVisualizer::EndEditing()
{
	CurrentCylinderSelectionIndex = INDEX_NONE;
	//GetEditedScoringComponent()->MarkRenderStateDirty();
	//GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableCylinderScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

UIntenSelectableCylinderScoring* FIntenSelectableCylinderScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableCylinderScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableCylinderScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                                 FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	UIntenSelectableCylinderScoring* ScoringComponent = GetEditedScoringComponent();

	if(ScoringComponent)
	{
		if (CurrentCylinderSelectionIndex != INDEX_NONE)
		{
			ScoringComponent->Modify();
		
			UE_LOG(LogTemp, Warning, TEXT("Current Delta: %s"), *DeltaTranslate.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Current Index: %d"), CurrentCylinderSelectionIndex);
		
			const FVector WorldSelection = ScoringComponent->GetComponentTransform().TransformPositionNoScale(ScoringComponent->LinePoints[CurrentCylinderSelectionIndex]);
			const FVector NewWorldPos = ScoringComponent->GetComponentTransform().InverseTransformPositionNoScale(WorldSelection + DeltaTranslate);
		
			ScoringComponent->LinePoints[CurrentCylinderSelectionIndex] += DeltaTranslate;
		
			UE_LOG(LogTemp, Warning, TEXT("Component: %s"), *(ScoringComponent->LinePoints[CurrentCylinderSelectionIndex]).ToString());
						
			ScoringComponent->MarkRenderStateDirty();
			GEditor->RedrawLevelEditingViewports(true);

			const FVector Average = (ScoringComponent->LinePoints[0] + ScoringComponent->LinePoints[1])/ 2;
			const FVector ShiftToMiddle = Average;

			ScoringComponent->SetWorldLocation(ScoringComponent->GetComponentTransform().TransformPositionNoScale(Average));
			ScoringComponent->LinePoints[0] -= ShiftToMiddle;
			ScoringComponent->LinePoints[1] -= ShiftToMiddle;

			TArray<FProperty*> Properties;
			Properties.Add(PointsProperty);
			Properties.Add(RadiusProperty);
			NotifyPropertiesModified(ScoringComponent, Properties, EPropertyChangeType::ValueSet);
		
			GEditor->RedrawLevelEditingViewports(true);
		
			bHandled = true;
		}else
		{
			//ScoringComponent->AddWorldOffset(DeltaTranslate);
			//ScoringComponent->AddWorldRotation(DeltaRotate);
			
			ScoringComponent->Modify();
			ScoringComponent->MarkRenderStateDirty();
			GEditor->RedrawLevelEditingViewports(true);
			
			UE_LOG(LogTemp, Warning, TEXT("Cylinder Selected!"));
			
			return false;
		}
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


