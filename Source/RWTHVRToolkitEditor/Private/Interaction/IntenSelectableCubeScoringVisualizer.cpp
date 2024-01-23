#include "Interaction/IntenSelectableCubeScoringVisualizer.h"

#include "DrawDebugHelpers.h"

#include "SceneManagement.h"
#include "MaterialShared.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCubeScoring.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialRenderProxy.h"

IMPLEMENT_HIT_PROXY(FCubeProxy, HComponentVisProxy);

FIntenSelectableCubeScoringVisualizer::FIntenSelectableCubeScoringVisualizer() : DebugMaterial(FColoredMaterialRenderProxy(GEngine->ConstraintLimitMaterial->GetRenderProxy(), FColor::Green))
{
	
}

FIntenSelectableCubeScoringVisualizer::~FIntenSelectableCubeScoringVisualizer()
{
}

FVector FIntenSelectableCubeScoringVisualizer::GetCurrentVectorWorld() const
{
	return CubeBehaviour->GetComponentLocation();
}

bool FIntenSelectableCubeScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	UE_LOG(LogTemp, Warning, TEXT("Handling Click"));
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FCubeProxy::StaticGetType()))
		{
			const UIntenSelectableCubeScoring* T = Cast<const UIntenSelectableCubeScoring>(VisProxy->Component.Get());
			CubeBehaviour = const_cast<UIntenSelectableCubeScoring*>(T);
			FCubeProxy* Proxy = (FCubeProxy*)VisProxy;
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

void FIntenSelectableCubeScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableCubeScoring* ComponentCasted = Cast<UIntenSelectableCubeScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		PDI->SetHitProxy(new FCubeProxy(Component, 0));
		const FVector Radii{ComponentCasted->XLength, ComponentCasted->YLength, ComponentCasted->ZLength};
		DrawBox(PDI, ComponentCasted->GetComponentTransform().ToMatrixNoScale(), Radii / 2, &DebugMaterial, 0);
		PDI->DrawPoint(ComponentCasted->GetComponentLocation(), FColor::Green, 20, 0);
		PDI->SetHitProxy(nullptr);
	}
}

void FIntenSelectableCubeScoringVisualizer::EndEditing()
{

}

UActorComponent* FIntenSelectableCubeScoringVisualizer::GetEditedComponent() const
{
	return CubeBehaviour;
}

bool FIntenSelectableCubeScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());

		const FVector LocalCenter = CubeBehaviour->GetComponentLocation();
		const FVector NewCenter = LocalCenter + DeltaTranslate;
		CubeBehaviour->SetWorldLocation(NewCenter);
		CubeBehaviour->AddWorldRotation(DeltaRotate);

		CubeBehaviour->XLength += DeltaScale.X * 3;
		CubeBehaviour->YLength += DeltaScale.Y * 3;
		CubeBehaviour->ZLength += DeltaScale.Z * 3;
		bHandled = true;
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
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


