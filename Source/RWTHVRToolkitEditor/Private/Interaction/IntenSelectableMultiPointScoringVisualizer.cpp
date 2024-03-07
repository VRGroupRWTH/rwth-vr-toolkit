#include "Interaction/IntenSelectableMultiPointScoringVisualizer.h"

#include "ActorEditorUtils.h"
#include "SceneManagement.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableMultiPointScoring.h"

IMPLEMENT_HIT_PROXY(FMultiPointProxy, HComponentVisProxy);

FIntenSelectableMultiPointScoringVisualizer::FIntenSelectableMultiPointScoringVisualizer()
{
	PointsProperty = FindFProperty<FProperty>(UIntenSelectableMultiPointScoring::StaticClass(), GET_MEMBER_NAME_CHECKED(UIntenSelectableMultiPointScoring, PointsToSelect));
}

FIntenSelectableMultiPointScoringVisualizer::~FIntenSelectableMultiPointScoringVisualizer()
{
}

FVector FIntenSelectableMultiPointScoringVisualizer::GetCurrentVectorWorld() const
{
	if(CurrentSelectionIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}
	return MultiPointBehaviour->GetComponentTransform().TransformPosition(MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex]);
}

bool FIntenSelectableMultiPointScoringVisualizer::IsVisualizingArchetype() const
{
	return (MultiPointBehaviour && MultiPointBehaviour->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(MultiPointBehaviour->GetOwner()));
}

bool FIntenSelectableMultiPointScoringVisualizer::ShowWhenSelected()
{
	return false;
}

bool FIntenSelectableMultiPointScoringVisualizer::ShouldShowForSelectedSubcomponents(const UActorComponent* Component)
{
	return false;
}

bool FIntenSelectableMultiPointScoringVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                                      HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;
	
	if (VisProxy && VisProxy->Component.IsValid())
	{
		bEditing = true;
		
		if (VisProxy->IsA(FMultiPointProxy::StaticGetType()))
		{
			const UIntenSelectableMultiPointScoring* T = Cast<const UIntenSelectableMultiPointScoring>(VisProxy->Component.Get());
			MultiPointBehaviour = const_cast<UIntenSelectableMultiPointScoring*>(T);
			FMultiPointProxy* Proxy = (FMultiPointProxy*)VisProxy;
			CurrentSelectionIndex = Proxy->TargetIndex;
			UE_LOG(LogTemp, Warning, TEXT("Handling Click %i"), CurrentSelectionIndex);
		}else
		{
			CurrentSelectionIndex = INDEX_NONE;
			UE_LOG(LogTemp, Warning, TEXT("Handling Click => no selection"));
		}
	}
	else
	{
		CurrentSelectionIndex = INDEX_NONE;
		UE_LOG(LogTemp, Warning, TEXT("Handling Click => no selection"));

	}

	return bEditing;
}


// Fill out your copyright notice in the Description page of Project Settings.

void FIntenSelectableMultiPointScoringVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
	const UIntenSelectableMultiPointScoring* ComponentCasted = Cast<UIntenSelectableMultiPointScoring>(Component);
	
	if (ComponentCasted != nullptr)
	{
		for(int i = 0; i < ComponentCasted->PointsToSelect.Num(); i++)
		{
			PDI->SetHitProxy(new FMultiPointProxy(Component, i));
			
			FVector PointWorld = ComponentCasted->GetComponentTransform().TransformPosition(ComponentCasted->PointsToSelect[i]);
			PDI->DrawPoint(PointWorld, FColor::Green, 20.f, SDPG_Foreground);
			
			PDI->SetHitProxy(nullptr);
		}
	}
}

void FIntenSelectableMultiPointScoringVisualizer::EndEditing()
{
	//MultiPointBehaviour->MarkRenderStateDirty();
	//GEditor->RedrawLevelEditingViewports(true);
}

UActorComponent* FIntenSelectableMultiPointScoringVisualizer::GetEditedComponent() const
{
	return MultiPointBehaviour;
}

bool FIntenSelectableMultiPointScoringVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                             FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (CurrentSelectionIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Selection! %s"), *DeltaTranslate.ToString());
		
		const FVector WorldSelection = MultiPointBehaviour->GetComponentTransform().TransformPosition(MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex]);
		const FVector NewWorldPos = MultiPointBehaviour->GetComponentTransform().InverseTransformPosition(WorldSelection + DeltaTranslate);
		MultiPointBehaviour->PointsToSelect[CurrentSelectionIndex] = NewWorldPos;
		
		MultiPointBehaviour->MarkRenderStateDirty();
		bHandled = true;
		//NotifyPropertyModified(MultiPointBehaviour, PointsProperty, EPropertyChangeType::Interactive);
		
		TArray<FProperty*> Properties;
		Properties.Add(PointsProperty);
		MyNotifyPropertiesModified(MultiPointBehaviour, Properties, EPropertyChangeType::Interactive);

		
		
		GEditor->RedrawLevelEditingViewports(true);

	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Current Selection!"));
	}

	return bHandled;
}

void FIntenSelectableMultiPointScoringVisualizer::MyNotifyPropertiesModified(UActorComponent* Component, const TArray<FProperty*>& Properties, EPropertyChangeType::Type PropertyChangeType)
{
	if (Component == nullptr)
	{
		return;
	}

	for (FProperty* Property : Properties)
	{
		FPropertyChangedEvent PropertyChangedEvent(Property, PropertyChangeType);
		Component->PostEditChangeProperty(PropertyChangedEvent);
	}

	AActor* Owner = Component->GetOwner();

	if (Owner && FActorEditorUtils::IsAPreviewOrInactiveActor(Owner))
	{
		// The component belongs to the preview actor in the BP editor, so we need to propagate the property change to the archetype.
		// Before this, we exploit the fact that the archetype and the preview actor have the old and new value of the property, respectively.
		// So we can go through all archetype instances, and if they hold the (old) archetype value, update it to the new value.

		// Get archetype
		UActorComponent* Archetype = Cast<UActorComponent>(Component->GetArchetype());
		check(Archetype);

		// Get all archetype instances (the preview actor passed in should be amongst them)
		TArray<UObject*> ArchetypeInstances;
		Archetype->GetArchetypeInstances(ArchetypeInstances);
		check(ArchetypeInstances.Contains(Component));

		// Identify which of the modified properties are at their default values in the archetype instances,
		// and thus need the new value to be propagated to them
		struct FInstanceDefaultProperties
		{
			UActorComponent* ArchetypeInstance;
			TArray<FProperty*, TInlineAllocator<8>> Properties;
		};

		TArray<FInstanceDefaultProperties> InstanceDefaultProperties;
		InstanceDefaultProperties.Reserve(ArchetypeInstances.Num());

		for (UObject* ArchetypeInstance : ArchetypeInstances)
		{
			UActorComponent* InstanceComp = Cast<UActorComponent>(ArchetypeInstance);
			if (InstanceComp != Component)
			{
				FInstanceDefaultProperties Entry;
				for (FProperty* Property : Properties)
				{
					uint8* ArchetypePtr = Property->ContainerPtrToValuePtr<uint8>(Archetype);
					uint8* InstancePtr = Property->ContainerPtrToValuePtr<uint8>(InstanceComp);
					if (Property->Identical(ArchetypePtr, InstancePtr))
					{
						Entry.Properties.Add(Property);
					}
				}

				if (Entry.Properties.Num() > 0)
				{
					Entry.ArchetypeInstance = InstanceComp;
					InstanceDefaultProperties.Add(MoveTemp(Entry));
				}
			}
		}

		// Propagate all modified properties to the archetype
		Archetype->SetFlags(RF_Transactional);
		Archetype->Modify();

		if (Archetype->GetOwner())
		{
			Archetype->GetOwner()->Modify();
		}

		for (FProperty* Property : Properties)
		{
			uint8* ArchetypePtr = Property->ContainerPtrToValuePtr<uint8>(Archetype);
			uint8* PreviewPtr = Property->ContainerPtrToValuePtr<uint8>(Component);
			Property->CopyCompleteValue(ArchetypePtr, PreviewPtr);

			FPropertyChangedEvent PropertyChangedEvent(Property);
			Archetype->PostEditChangeProperty(PropertyChangedEvent);
		}

		// Apply changes to each archetype instance
		for (const auto& Instance : InstanceDefaultProperties)
		{
			Instance.ArchetypeInstance->SetFlags(RF_Transactional);
			Instance.ArchetypeInstance->Modify();

			AActor* InstanceOwner = Instance.ArchetypeInstance->GetOwner();

			if (InstanceOwner)
			{
				InstanceOwner->Modify();
			}

			for (FProperty* Property : Instance.Properties)
			{
				uint8* InstancePtr = Property->ContainerPtrToValuePtr<uint8>(Instance.ArchetypeInstance);
				uint8* PreviewPtr = Property->ContainerPtrToValuePtr<uint8>(Component);
				Property->CopyCompleteValue(InstancePtr, PreviewPtr);

				FPropertyChangedEvent PropertyChangedEvent(Property);
				Instance.ArchetypeInstance->PostEditChangeProperty(PropertyChangedEvent);
			}

			// Rerun construction script on instance
			if (InstanceOwner)
			{
				InstanceOwner->PostEditMove(PropertyChangeType == EPropertyChangeType::ValueSet);
			}
		}
	}

	// Rerun construction script on preview actor
	if (Owner)
	{
		Owner->PostEditMove(PropertyChangeType == EPropertyChangeType::ValueSet);
	}
}

bool FIntenSelectableMultiPointScoringVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (MultiPointBehaviour && CurrentSelectionIndex != INDEX_NONE)
	{
		OutLocation = GetCurrentVectorWorld();
        
		return true;
	}

	return false;
}


