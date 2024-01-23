#include "Interaction/IntenSelectableCircleScoringVisualizer.h"

#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCircleScoring.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_HIT_PROXY(FCircleProxy, HComponentVisProxy);

FIntenSelectableCircleScoringVisualizer::FIntenSelectableCircleScoringVisualizer()
{
	
}

FIntenSelectableCircleScoringVisualizer::~FIntenSelectableCircleScoringVisualizer()
{
}

FVector FIntenSelectableCircleScoringVisualizer::GetCurrentVectorWorld() const
{
	switch (CurrentSelectionIndex)
	{
		case 0:
			return CircleBehaviour->GetComponentLocation();
		case 1:
			{
				const FVector CenterWorld = CircleBehaviour->GetComponentLocation();
				const FVector NormalWorldPoint = CircleBehaviour->GetComponentTransform().TransformPosition(FVector::ForwardVector);
				const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
				const FVector Y = WorldNormalDir.RotateAngleAxis(90, CircleBehaviour->GetRightVector()).GetSafeNormal() * CircleBehaviour->Radius;
				return CenterWorld + Y;
			}
	default:
		return FVector::ZeroVector;
	}
}

bool FIntenSelectableCircleScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FCircleProxy::StaticGetType()))
		{
			const UIntenSelectableCircleScoring* T = Cast<const UIntenSelectableCircleScoring>(VisProxy->Component.Get());
			CircleBehaviour = const_cast<UIntenSelectableCircleScoring*>(T);
			FCircleProxy* Proxy = (FCircleProxy*)VisProxy;
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

void FIntenSelectableCircleScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableCircleScoring* ComponentCasted = Cast<UIntenSelectableCircleScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		PDI->SetHitProxy(new FCircleProxy(Component, 0));
		const FVector CenterWorld = ComponentCasted->GetComponentLocation();
		PDI->DrawPoint(CenterWorld, FColor::Green, 20.f, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
		
		PDI->SetHitProxy(new FCircleProxy(Component, 1));
		const FVector NormalWorldPoint = ComponentCasted->GetComponentTransform().TransformPosition(FVector::ForwardVector);
		const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
		const FVector Y = WorldNormalDir.RotateAngleAxis(90, ComponentCasted->GetRightVector());
		const FVector Z = FVector::CrossProduct(Y.GetSafeNormal(), WorldNormalDir);
		DrawCircle(PDI, CenterWorld, Y.GetSafeNormal(), Z.GetSafeNormal(), FColor::Green, ComponentCasted->Radius, 100, SDPG_Foreground, 2);
		PDI->SetHitProxy(nullptr);

	}
}

void FIntenSelectableCircleScoringVisualizer::EndEditing()
{

}

UActorComponent* FIntenSelectableCircleScoringVisualizer::GetEditedComponent() const
{
	return CircleBehaviour;
}

bool FIntenSelectableCircleScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		if(CurrentSelectionIndex == 0)
		{
			const FVector LocalCenter = CircleBehaviour->GetComponentLocation();
			const FVector NewCenter = LocalCenter + DeltaTranslate;
			CircleBehaviour->SetWorldLocation(NewCenter);
			CircleBehaviour->AddWorldRotation(DeltaRotate);
			
			bHandled = true;
		}else if(CurrentSelectionIndex == 1)
		{
			const FVector CenterWorld = CircleBehaviour->GetComponentLocation();
			const FVector NormalWorldPoint = CircleBehaviour->GetComponentTransform().TransformPosition(FVector::ForwardVector);
			const FVector WorldNormalDir = NormalWorldPoint - CenterWorld;
			const FVector RadiusVector = WorldNormalDir.RotateAngleAxis(90, CircleBehaviour->GetRightVector()).GetSafeNormal() * CircleBehaviour->Radius;

			const FVector ClampedTranslate = DeltaTranslate.Size() > 100 ? DeltaTranslate.GetSafeNormal() * 100 : DeltaTranslate;
			CircleBehaviour->Radius = FVector::Distance(CenterWorld, CenterWorld + RadiusVector + ClampedTranslate);
			bHandled = true;
		}
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
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


