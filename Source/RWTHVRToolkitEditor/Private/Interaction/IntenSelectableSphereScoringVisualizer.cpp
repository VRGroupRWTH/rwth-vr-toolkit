#include "Interaction/IntenSelectableSphereScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "SceneManagement.h"
#include "MaterialShared.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableSphereScoring.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"

IMPLEMENT_HIT_PROXY(FSphereProxy, HComponentVisProxy);

FIntenSelectableSphereScoringVisualizer::FIntenSelectableSphereScoringVisualizer() : DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableSphereScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableSphereScoring, Radius));
}

FIntenSelectableSphereScoringVisualizer::~FIntenSelectableSphereScoringVisualizer()
{
}

bool FIntenSelectableSphereScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());
}

FVector FIntenSelectableSphereScoringVisualizer::GetCurrentVectorWorld() const
{
	return GetEditedScoringComponent()->GetComponentLocation();
}

bool FIntenSelectableSphereScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableSphereScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableSphereScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                  HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	//UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FSphereProxy::StaticGetType()))
		{
			const UIntenSelectableSphereScoring* T = Cast<const UIntenSelectableSphereScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);
			
			FSphereProxy* Proxy = (FSphereProxy*)VisProxy;
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

void FIntenSelectableSphereScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableSphereScoring* ComponentCasted = Cast<UIntenSelectableSphereScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		PDI->SetHitProxy(new FSphereProxy(Component, 0));
		
		DrawSphere(PDI, ComponentCasted->GetComponentLocation(), FRotator::ZeroRotator, FVector::OneVector * ComponentCasted->Radius, 50, 50, &DebugMaterial, 0, false);

		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableSphereScoringVisualizer::EndEditing()
{

}

UActorComponent* FIntenSelectableSphereScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

UIntenSelectableSphereScoring* FIntenSelectableSphereScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableSphereScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableSphereScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                               FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (true || CurrentSelectionIndex != INDEX_NONE)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		if((true || CurrentSelectionIndex == 0 )&& GetEditedComponent())
		{
			UIntenSelectableSphereScoring* ScoringComponent = GetEditedScoringComponent();
			ScoringComponent->Modify();
			
			const FVector LocalCenter = ScoringComponent->GetComponentLocation();
			const FVector NewCenter = LocalCenter + DeltaTranslate;
			
			ScoringComponent->SetWorldLocation(NewCenter);
			ScoringComponent->AddWorldRotation(DeltaRotate);
			
			const float AverageScaleFactor = (DeltaScale.X + DeltaScale.Y + DeltaScale.Z) / 3.0f;

			// Apply the average scale factor to the original radius
			float NewRadius;
			if (AverageScaleFactor > 0)
			{
				// Scale up: Increase the radius
				NewRadius = ScoringComponent->Radius * (1.0f + FMath::Abs(AverageScaleFactor));
			}
			else
			{
				// Scale down: Decrease the radius, ensuring not to reduce it below a minimum threshold (e.g., not making it negative)
				NewRadius = ScoringComponent->Radius * FMath::Max(0.1f, 1.0f + AverageScaleFactor);
			}
			ScoringComponent->Radius = NewRadius;

			TArray<FProperty*> Properties;
			Properties.Add(PointsProperty);
			NotifyPropertiesModified(ScoringComponent, Properties, EPropertyChangeType::ValueSet);

			ScoringComponent->MarkRenderStateDirty();
			GEditor->RedrawLevelEditingViewports(false);
			bHandled = true;
		}
	}else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableSphereScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


