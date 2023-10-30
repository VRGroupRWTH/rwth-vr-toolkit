#include "RWTHVRToolkitEditor.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Interaction/GrabbingBehaviorOnLineComponent.h"
#include "Interaction/GrabbingBehaviorOnLineVisualizer.h"
#include "Interaction/GrabbingBehaviorOnPlaneComponent.h"
#include "Interaction/GrabbingBehaviorPlaneVisualizer.h"

IMPLEMENT_GAME_MODULE(FRWTHVRToolkitEditorModule, RWTHVRToolkitEditor);

#define LOCTEXT_NAMESPACE "RWTHVRToolkitEditor"

void FRWTHVRToolkitEditorModule::StartupModule()
{
	if (GUnrealEd != nullptr)
	{
		const TSharedPtr<FComponentVisualizer> LineVisualizer = MakeShareable(new FGrabbingBehaviorOnLineVisualizer());

		if (LineVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UGrabbingBehaviorOnLineComponent::StaticClass()->GetFName(),
			                                       LineVisualizer);
			LineVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> PlaneVisualizer = MakeShareable(new FGrabbingBehaviorPlaneVisualizer());

		if (PlaneVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UGrabbingBehaviorOnPlaneComponent::StaticClass()->GetFName(),
			                                       PlaneVisualizer);
			PlaneVisualizer->OnRegister();
		}
	}
}

void FRWTHVRToolkitEditorModule::ShutdownModule()
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UGrabbingBehaviorOnLineComponent::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UGrabbingBehaviorOnPlaneComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
