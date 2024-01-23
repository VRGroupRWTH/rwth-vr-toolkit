#include "Interaction/IntenSelectableRectangleScoringVisualizer.h"

#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableRectangleScoring.h"
#include "Kismet/KismetMathLibrary.h"

IMPLEMENT_HIT_PROXY(FRectangleProxy, HComponentVisProxy);

FIntenSelectableRectangleScoringVisualizer::FIntenSelectableRectangleScoringVisualizer()
{
	
}

FIntenSelectableRectangleScoringVisualizer::~FIntenSelectableRectangleScoringVisualizer()
{
}

FVector FIntenSelectableRectangleScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}

	const FVector X = RectangleBehaviour->GetRightVector() * RectangleBehaviour->XLength;
	const FVector Y = RectangleBehaviour->GetUpVector() * RectangleBehaviour->YLength;

	const FVector CornerWorld00 = RectangleBehaviour->GetComponentTransform().TransformPosition(FVector::ZeroVector) - (X / 2) - (Y / 2);
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

bool FIntenSelectableRectangleScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FRectangleProxy::StaticGetType()))
		{
			const UIntenSelectableRectangleScoring* T = Cast<const UIntenSelectableRectangleScoring>(VisProxy->Component.Get());
			RectangleBehaviour = const_cast<UIntenSelectableRectangleScoring*>(T);
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
	return RectangleBehaviour;
}

bool FIntenSelectableRectangleScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (CurrentSelectionIndex != INDEX_NONE)
	{
		switch (CurrentSelectionIndex)
		{
		case 0:
			//bottom
			RectangleBehaviour->YLength -= DeltaTranslate.Z;
			if(RectangleBehaviour->YLength < 0.1)
			{
				RectangleBehaviour->YLength = 0.1;
			}else
			{
				RectangleBehaviour->AddLocalOffset(FVector::UpVector * DeltaTranslate.Z / 2);
			}
			return true;
		case 1:
			//left
			RectangleBehaviour->XLength -= DeltaTranslate.Y;
			if(RectangleBehaviour->XLength < 0.1)
			{
				RectangleBehaviour->XLength = 0.1;
			}else
			{
				RectangleBehaviour->AddLocalOffset(FVector::RightVector * DeltaTranslate.Y / 2);
			}
			return true;
		case 2:
			//top
			RectangleBehaviour->YLength += DeltaTranslate.Z;
			if(RectangleBehaviour->YLength < 0.1)
			{
				RectangleBehaviour->YLength = 0.1;
			}else
			{
				RectangleBehaviour->AddLocalOffset(FVector::UpVector * DeltaTranslate.Z / 2);
			}
			return true;
		case 3:
			//right
			RectangleBehaviour->XLength += DeltaTranslate.Y;
			if(RectangleBehaviour->XLength < 0.1)
			{
				RectangleBehaviour->XLength = 0.1;
			}else
			{
				RectangleBehaviour->AddLocalOffset(FVector::RightVector * DeltaTranslate.Y / 2);
			}
			return true;
		case 4:
			{
				//middle
				const FVector WorldCenter = RectangleBehaviour->GetComponentLocation();
				const FVector NewWorldCenter = WorldCenter + DeltaTranslate;
				RectangleBehaviour->SetWorldLocation(NewWorldCenter);
				RectangleBehaviour->AddWorldRotation(DeltaRotate);
				
				const FVector LocalScaleDelta = RectangleBehaviour->GetComponentTransform().InverseTransformVector(DeltaScale);
				RectangleBehaviour->XLength += LocalScaleDelta.Y * 4;
				RectangleBehaviour->YLength += LocalScaleDelta.Z * 4;
				return true;
			}
		default:
			return false;
		}
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
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


