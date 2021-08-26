#include "RWTHVRToolkitEditor.h"

#include "ComponentVisualizers.h"
#include "Interaction/GrabbingBehaviorOnLineVisualizer.h"
#include "Interaction/GrabbingBehaviorPlaneVisualizer.h"

#include "Interaction/GrabbingBehaviorOnPlaneComponent.h"
#include "Interaction/GrabbingBehaviorOnLineComponent.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

IMPLEMENT_GAME_MODULE(FRWTHVRToolkitEditorModule, RWTHVRToolkitEditor);

#define LOCTEXT_NAMESPACE "RWTHVRToolkitEditor"

void FRWTHVRToolkitEditorModule::StartupModule()
{
	if (GUnrealEd != NULL)
	{
		TSharedPtr<FComponentVisualizer> LineVisualizer = MakeShareable(new FGrabbingBehaviorOnLineVisualizer());

		if (LineVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UGrabbingBehaviorOnLineComponent::StaticClass()->GetFName(), LineVisualizer);
			LineVisualizer->OnRegister();
		}

		TSharedPtr<FComponentVisualizer> PlaneVisualizer = MakeShareable(new FGrabbingBehaviorPlaneVisualizer());
		
		if (PlaneVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UGrabbingBehaviorOnPlaneComponent::StaticClass()->GetFName(), PlaneVisualizer);
			PlaneVisualizer->OnRegister();
		}
	}
}

void FRWTHVRToolkitEditorModule::ShutdownModule()
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->UnregisterComponentVisualizer(UGrabbingBehaviorOnLineComponent::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UGrabbingBehaviorOnPlaneComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE