#include "Interaction/IntenSelectableSphereScoringVisualizer.h"
#include "SceneManagement.h"
#include "MaterialShared.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableSphereScoring.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"

IMPLEMENT_HIT_PROXY(FSphereProxy, HComponentVisProxy);

FIntenSelectableSphereScoringVisualizer::FIntenSelectableSphereScoringVisualizer() : DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{
	
}

FIntenSelectableSphereScoringVisualizer::~FIntenSelectableSphereScoringVisualizer()
{
}

FVector FIntenSelectableSphereScoringVisualizer::GetCurrentVectorWorld() const
{
	return SphereBehaviour->GetComponentLocation();
}

bool FIntenSelectableSphereScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FSphereProxy::StaticGetType()))
		{
			const UIntenSelectableSphereScoring* T = Cast<const UIntenSelectableSphereScoring>(VisProxy->Component.Get());
			SphereBehaviour = const_cast<UIntenSelectableSphereScoring*>(T);
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


// Fill out your copyright notice in the Description page of Project Settings.

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
	return SphereBehaviour;
}

bool FIntenSelectableSphereScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		if(CurrentSelectionIndex == 0)
		{
			const FVector LocalCenter = SphereBehaviour->GetComponentLocation();
			const FVector NewCenter = LocalCenter + DeltaTranslate;
			SphereBehaviour->SetWorldLocation(NewCenter);
			SphereBehaviour->AddWorldRotation(DeltaRotate);
			SphereBehaviour->Radius += (DeltaScale.X + DeltaScale.Y + DeltaScale.Z) * 2;
			
			bHandled = true;
		}
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
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


