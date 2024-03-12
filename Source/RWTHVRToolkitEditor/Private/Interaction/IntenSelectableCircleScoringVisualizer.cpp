#include "Interaction/IntenSelectableCircleScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCircleScoring.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_HIT_PROXY(FCircleProxy, HComponentVisProxy);

FIntenSelectableCircleScoringVisualizer::FIntenSelectableCircleScoringVisualizer()
{
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableCircleScoring::StaticClass(),
											  GET_MEMBER_NAME_CHECKED(UIntenSelectableCircleScoring, Radius));
}

FIntenSelectableCircleScoringVisualizer::~FIntenSelectableCircleScoringVisualizer() {}

FVector FIntenSelectableCircleScoringVisualizer::GetCurrentVectorWorld() const
{
	switch (CurrentSelectionIndex)
	{
	case 0:
		return GetEditedScoringComponent()->GetComponentLocation();
	case 1:
		{
			const FVector CenterWorld = GetEditedScoringComponent()->GetComponentLocation();
			const FVector NormalWorldPoint =
				GetEditedScoringComponent()->GetComponentTransform().TransformPosition(FVector::ForwardVector);
			const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
			const FVector Y =
				WorldNormalDir.RotateAngleAxis(90, GetEditedScoringComponent()->GetRightVector()).GetSafeNormal() *
				GetEditedScoringComponent()->Radius;
			return CenterWorld + Y;
		}
	default:
		return FVector::ZeroVector;
	}
}

bool FIntenSelectableCircleScoringVisualizer::IsVisualizingArchetype() const
{
	return GetEditedScoringComponent() && GetEditedScoringComponent()->GetOwner() &&
		FActorEditorUtils::IsAPreviewOrInactiveActor(GetEditedScoringComponent()->GetOwner());
}

UIntenSelectableCircleScoring* FIntenSelectableCircleScoringVisualizer::GetEditedScoringComponent() const
{
	return Cast<UIntenSelectableCircleScoring>(ScoringBehaviourPropertyPath.GetComponent());
}

bool FIntenSelectableCircleScoringVisualizer::ShowWhenSelected() { return false; }

bool FIntenSelectableCircleScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableCircleScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
																  HComponentVisProxy* VisProxy,
																  const FViewportClick& Click)
{
	bool bEditing = false;

	// UE_LOG(LogTemp, Warning, TEXT("Handling Click"));

	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;

		if (VisProxy->IsA(FCircleProxy::StaticGetType()))
		{
			const UIntenSelectableCircleScoring* T =
				Cast<const UIntenSelectableCircleScoring>(VisProxy->Component.Get());
			ScoringBehaviourPropertyPath = FComponentPropertyPath(T);

			FCircleProxy* Proxy = (FCircleProxy*)VisProxy;
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

void FIntenSelectableCircleScoringVisualizer::DrawVisualization(const UActorComponent* Component,
																const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UIntenSelectableCircleScoring* ComponentCasted = Cast<UIntenSelectableCircleScoring>(Component);

	if (ComponentCasted != nullptr)
	{
		PDI->SetHitProxy(new FCircleProxy(Component, 0));

		const FVector CenterWorld = ComponentCasted->GetComponentLocation();
		PDI->DrawPoint(CenterWorld, FColor::Green, 20.f, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);

		PDI->SetHitProxy(new FCircleProxy(Component, 1));
		const FVector NormalWorldPoint =
			ComponentCasted->GetComponentTransform().TransformPosition(FVector::ForwardVector);
		const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
		const FVector Y = WorldNormalDir.RotateAngleAxis(90, ComponentCasted->GetRightVector());
		const FVector Z = FVector::CrossProduct(Y.GetSafeNormal(), WorldNormalDir);
		DrawCircle(PDI, CenterWorld, Y.GetSafeNormal(), Z.GetSafeNormal(), FColor::Green, ComponentCasted->Radius, 100,
				   SDPG_Foreground, 2);

		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableCircleScoringVisualizer::EndEditing() {}

UActorComponent* FIntenSelectableCircleScoringVisualizer::GetEditedComponent() const
{
	return GetEditedScoringComponent();
}

bool FIntenSelectableCircleScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient,
															   FViewport* Viewport, FVector& DeltaTranslate,
															   FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		if (CurrentSelectionIndex == 0)
		{
			const FVector LocalCenter = GetEditedScoringComponent()->GetComponentLocation();
			const FVector NewCenter = LocalCenter + DeltaTranslate;
			GetEditedScoringComponent()->SetWorldLocation(NewCenter);
			GetEditedScoringComponent()->AddWorldRotation(DeltaRotate);

			bHandled = true;
		}
		else if (CurrentSelectionIndex == 1)
		{
			const FVector CenterWorld = GetEditedScoringComponent()->GetComponentLocation();
			const FVector NormalWorldPoint =
				GetEditedScoringComponent()->GetComponentTransform().TransformPosition(FVector::ForwardVector);
			const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
			const FVector RadiusVector =
				WorldNormalDir.RotateAngleAxis(90, GetEditedScoringComponent()->GetRightVector()).GetSafeNormal() *
				GetEditedScoringComponent()->Radius;

			const FVector ClampedTranslate =
				DeltaTranslate.Size() > 100 ? DeltaTranslate.GetSafeNormal() * 100 : DeltaTranslate;
			GetEditedScoringComponent()->Radius =
				FVector::Distance(CenterWorld, CenterWorld + RadiusVector + ClampedTranslate);
			bHandled = true;
		}

		TArray<FProperty*> Properties;
		Properties.Add(PointsProperty);
		NotifyPropertiesModified(GetEditedScoringComponent(), Properties, EPropertyChangeType::ValueSet);
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

bool FIntenSelectableCircleScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
																FVector& OutLocation) const
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();

		return true;
	}

	return false;
}
