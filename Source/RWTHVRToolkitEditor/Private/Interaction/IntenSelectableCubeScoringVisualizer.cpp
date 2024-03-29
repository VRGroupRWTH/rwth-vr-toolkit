#include "Interaction/IntenSelectableCubeScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "DrawDebugHelpers.h"
#include "SceneManagement.h"
#include "MaterialShared.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCubeScoring.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialRenderProxy.h"

IMPLEMENT_HIT_PROXY(FCubeProxy, HComponentVisProxy);

FIntenSelectableCubeScoringVisualizer::FIntenSelectableCubeScoringVisualizer() :
	DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{
}

FIntenSelectableCubeScoringVisualizer::~FIntenSelectableCubeScoringVisualizer() {}

FVector FIntenSelectableCubeScoringVisualizer::GetCurrentVectorWorld() const
{
	return GetEditedScoringComponent()->GetComponentLocation();
}

bool FIntenSelectableCubeScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() &&
		FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());
}

UIntenSelectableCubeScoring* FIntenSelectableCubeScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableCubeScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableCubeScoringVisualizer::ShowWhenSelected() { return false; }

bool FIntenSelectableCubeScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableCubeScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
																HComponentVisProxy* VisProxy,
																const FViewportClick& Click)
{
	bool bEditing = false;

	// UE_LOG(LogTemp, Warning, TEXT("Handling Click"));

	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;

		if (VisProxy->IsA(FCubeProxy::StaticGetType()))
		{
			const UIntenSelectableCubeScoring* T = Cast<const UIntenSelectableCubeScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);

			FCubeProxy* Proxy = (FCubeProxy*)VisProxy;
			CurrentSelectionIndex = Proxy->TargetIndex;
		}
		else
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

void FIntenSelectableCubeScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
															  FPrimitiveDrawInterface* PDI)
{
	const UIntenSelectableCubeScoring* ComponentCasted = Cast<UIntenSelectableCubeScoring>(Component);

	if (ComponentCasted != nullptr)
	{
		PDI->SetHitProxy(new FCubeProxy(Component, 0));

		const auto Scale = ComponentCasted->GetRelativeTransform().GetScale3D();
		const FVector Radii{Scale.X, Scale.Y, Scale.Z};
		DrawBox(PDI, ComponentCasted->GetComponentTransform().ToMatrixNoScale(), Radii / 2, &DebugMaterial, 0);
		PDI->DrawPoint(ComponentCasted->GetComponentLocation(), FColor::Green, 20, 0);

		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableCubeScoringVisualizer::EndEditing() {}

UActorComponent* FIntenSelectableCubeScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

bool FIntenSelectableCubeScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
															 FVector& DeltaTranslate, FRotator& DeltaRotate,
															 FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		const FVector LocalCenter = GetEditedScoringComponent()->GetComponentLocation();
		const FVector NewCenter = LocalCenter + DeltaTranslate;
		GetEditedScoringComponent()->SetWorldLocation(NewCenter);
		GetEditedScoringComponent()->AddWorldRotation(DeltaRotate);

		auto Scale = GetEditedScoringComponent()->GetRelativeTransform().GetScale3D();

		Scale.X += DeltaScale.X * 3;
		Scale.Y += DeltaScale.Y * 3;
		Scale.Z += DeltaScale.Z * 3;

		GetEditedScoringComponent()->GetRelativeTransform().SetScale3D(Scale);
		bHandled = true;
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableCubeScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
															  FVector& OutLocation) const
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();

		return true;
	}

	return false;
}
