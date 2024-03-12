#include "ActorEditorUtils.h"
#include "Interaction/IntenSelectableRectangleScoringVisualizer.h"
#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableRectangleScoring.h"

IMPLEMENT_HIT_PROXY(FRectangleProxy, HComponentVisProxy);

FIntenSelectableRectangleScoringVisualizer::FIntenSelectableRectangleScoringVisualizer()
{
	XLengthProperty = FindFProperty<FProperty>(UIntenSelectableRectangleScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableRectangleScoring, XLength));
	YLengthProperty = FindFProperty<FProperty>(UIntenSelectableRectangleScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableRectangleScoring, YLength));
}

FIntenSelectableRectangleScoringVisualizer::~FIntenSelectableRectangleScoringVisualizer()
{
}

bool FIntenSelectableRectangleScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());

}

FVector FIntenSelectableRectangleScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}

	const FVector X = GetEditedScoringComponent()->GetRightVector() * GetEditedScoringComponent()->XLength;
	const FVector Y = GetEditedScoringComponent()->GetUpVector() * GetEditedScoringComponent()->YLength;

	const FVector CornerWorld00 = GetEditedScoringComponent()->GetComponentTransform().TransformPosition(FVector::ZeroVector) - (X / 2) - (Y / 2);
	const FVector CornerWorld10 = CornerWorld00 + X;
	const FVector CornerWorld01 = CornerWorld00 + Y;
	const FVector CornerWorld11 = CornerWorld00 + X + Y;


	switch (CurrentSelectionIndex)
	{
	case 0:
		//bottom
		return CornerWorld00 + (CornerWorld10 - CornerWorld00) * 0.5;
	case 1:
		//left
		return CornerWorld00 + (CornerWorld01 - CornerWorld00) * 0.5;
	case 2:
		//top
		return CornerWorld01 + (CornerWorld11 - CornerWorld01) * 0.5;
	case 3:
		//right
		return CornerWorld11 + (CornerWorld10 - CornerWorld11) * 0.5;
	case 4:
		//middle
		return CornerWorld00 + ((CornerWorld10 - CornerWorld00) * 0.5) - ((CornerWorld10 - CornerWorld11) * 0.5);
	default:
		return FVector::ZeroVector;
	}
}

bool FIntenSelectableRectangleScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableRectangleScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableRectangleScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FRectangleProxy::StaticGetType()))
		{
			const UIntenSelectableRectangleScoring* T = Cast<const UIntenSelectableRectangleScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);
			
			FRectangleProxy* Proxy = (FRectangleProxy*)VisProxy;
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

void FIntenSelectableRectangleScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableRectangleScoring* ComponentCasted = Cast<UIntenSelectableRectangleScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		const FVector X = ComponentCasted->GetRightVector() * ComponentCasted->XLength;
		const FVector Y = ComponentCasted->GetUpVector() * ComponentCasted->YLength;

		const FVector CornerWorld00 = ComponentCasted->GetComponentTransform().TransformPosition(FVector::ZeroVector) - (X / 2) - (Y / 2);
		const FVector CornerWorld10 = CornerWorld00 + X;
		const FVector CornerWorld01 = CornerWorld00 + Y;
		const FVector CornerWorld11 = CornerWorld00 + X + Y;

		//bottom 0
		PDI->SetHitProxy(new FRectangleProxy(Component, 0));
		PDI->DrawLine(CornerWorld00, CornerWorld10, FColor::Green, SDPG_Foreground, 3);
		PDI->SetHitProxy(nullptr);

		//left 1
		PDI->SetHitProxy(new FRectangleProxy(Component, 1));
		PDI->DrawLine(CornerWorld00, CornerWorld01, FColor::Green, SDPG_Foreground, 3);
		PDI->SetHitProxy(nullptr);

		//up 2
		PDI->SetHitProxy(new FRectangleProxy(Component, 2));
		PDI->DrawLine(CornerWorld01, CornerWorld11, FColor::Green, SDPG_Foreground, 3);
		PDI->SetHitProxy(nullptr);


		//right 3
		PDI->SetHitProxy(new FRectangleProxy(Component, 3));
		PDI->DrawLine(CornerWorld11, CornerWorld10, FColor::Green, SDPG_Foreground, 3);
		PDI->SetHitProxy(nullptr);

		//middle
		PDI->SetHitProxy(new FRectangleProxy(Component, 4));
		const FVector Middle = CornerWorld00 + ((CornerWorld10 - CornerWorld00) * 0.5) - ((CornerWorld10 - CornerWorld11) * 0.5);
		PDI->DrawPoint(Middle, FColor::Green, 20, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableRectangleScoringVisualizer::EndEditing()
{
	GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableRectangleScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

UIntenSelectableRectangleScoring* FIntenSelectableRectangleScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableRectangleScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableRectangleScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                                  FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;
	
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UIntenSelectableRectangleScoring* ScoringComponent = GetEditedScoringComponent();
		ScoringComponent->Modify();
		
		switch (CurrentSelectionIndex)
		{
		case 0:
			//bottom
			ScoringComponent->YLength -= DeltaTranslate.Z;
			if(ScoringComponent->YLength < 0.1)
			{
				ScoringComponent->YLength = 0.1;
			}else
			{
				ScoringComponent->AddLocalOffset(FVector::UpVector * DeltaTranslate.Z / 2);
			}
			bHandled = true;
			break;
		case 1:
			//left
			ScoringComponent->XLength -= DeltaTranslate.Y;
			if(ScoringComponent->XLength < 0.1)
			{
				ScoringComponent->XLength = 0.1;
			}else
			{
				ScoringComponent->AddLocalOffset(FVector::RightVector * DeltaTranslate.Y / 2);
			}
			bHandled = true;
			break;
		case 2:
			//top
			ScoringComponent->YLength += DeltaTranslate.Z;
			if(ScoringComponent->YLength < 0.1)
			{
				ScoringComponent->YLength = 0.1;
			}else
			{
				ScoringComponent->AddLocalOffset(FVector::UpVector * DeltaTranslate.Z / 2);
			}
			bHandled = true;
			break;
		case 3:
			//right
			ScoringComponent->XLength += DeltaTranslate.Y;
			if(ScoringComponent->XLength < 0.1)
			{
				ScoringComponent->XLength = 0.1;
			}else
			{
				ScoringComponent->AddLocalOffset(FVector::RightVector * DeltaTranslate.Y / 2);
			}
			bHandled = true;
			break;
		case 4:
			{
				//middle
				const FVector WorldCenter = ScoringComponent->GetComponentLocation();
				const FVector NewWorldCenter = WorldCenter + DeltaTranslate;
				ScoringComponent->SetWorldLocation(NewWorldCenter);
				ScoringComponent->AddWorldRotation(DeltaRotate);
				
				const FVector LocalScaleDelta = ScoringComponent->GetComponentTransform().InverseTransformVector(DeltaScale);
				ScoringComponent->XLength += LocalScaleDelta.Y * 4;
				ScoringComponent->YLength += LocalScaleDelta.Z * 4;
				bHandled = true;
				break;
			}
		default:
			bHandled = false;
			break;
		}

		TArray<FProperty*> Properties;
		Properties.Add(XLengthProperty);
		Properties.Add(YLengthProperty);
		NotifyPropertiesModified(ScoringComponent, Properties, EPropertyChangeType::ValueSet);
		
		return bHandled;
	}else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return false;
}

bool FIntenSelectableRectangleScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


