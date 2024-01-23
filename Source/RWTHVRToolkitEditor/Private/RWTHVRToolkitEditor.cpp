#include "RWTHVRToolkitEditor.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Interaction/IntenSelectableCylinderScoringVisualizer.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCircleScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCubeScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableCylinderScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableLineScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableMultiPointScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableRectangleScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableSphereScoring.h"

IMPLEMENT_GAME_MODULE(FRWTHVRToolkitEditorModule, RWTHVRToolkitEditor);

#include "RWTHVRToolkitEditor.h"
#include "ComponentVisualizers.h"
#include "Interaction/IntenSelectableLineScoringVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Interaction/IntenSelectableCircleScoringVisualizer.h"
#include "Interaction/IntenSelectableCubeScoringVisualizer.h"
#include "Interaction/IntenSelectableMultiPointScoringVisualizer.h"
#include "Interaction/IntenSelectableRectangleScoringVisualizer.h"
#include "Interaction/IntenSelectableSphereScoringVisualizer.h"

#define LOCTEXT_NAMESPACE "RWTHVRToolkitEditor"

void FRWTHVRToolkitEditorModule::StartupModule()
{
	if (GUnrealEd != NULL)
	{
		const TSharedPtr<FComponentVisualizer> IntenSelectableLineVisualizer = MakeShareable(new FIntenSelectableLineScoringVisualizer());
		
		if (IntenSelectableLineVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableLineScoring::StaticClass()->GetFName(), IntenSelectableLineVisualizer);
			IntenSelectableLineVisualizer->OnRegister();
		}

		
		const TSharedPtr<FComponentVisualizer> IntenSelectableMultipointScoringVisualizer = MakeShareable(new FIntenSelectableMultiPointScoringVisualizer());
		
		if (IntenSelectableMultipointScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableMultiPointScoring::StaticClass()->GetFName(), IntenSelectableMultipointScoringVisualizer);
			IntenSelectableMultipointScoringVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> IntenSelectableCircleScoringVisualizer = MakeShareable(new FIntenSelectableCircleScoringVisualizer());
		
		if (IntenSelectableCircleScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableCircleScoring::StaticClass()->GetFName(), IntenSelectableCircleScoringVisualizer);
			IntenSelectableCircleScoringVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> IntenSelectableRectangleScoringVisualizer = MakeShareable(new FIntenSelectableRectangleScoringVisualizer());
		
		if (IntenSelectableRectangleScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableRectangleScoring::StaticClass()->GetFName(), IntenSelectableRectangleScoringVisualizer);
			IntenSelectableRectangleScoringVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> IntenSelectableSphereScoringVisualizer = MakeShareable(new FIntenSelectableSphereScoringVisualizer());
		
		if (IntenSelectableSphereScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableSphereScoring::StaticClass()->GetFName(), IntenSelectableSphereScoringVisualizer);
			IntenSelectableSphereScoringVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> IntenSelectableCubeScoringVisualizer = MakeShareable(new FIntenSelectableCubeScoringVisualizer());
		
		if (IntenSelectableCubeScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableCubeScoring::StaticClass()->GetFName(), IntenSelectableCubeScoringVisualizer);
			IntenSelectableCubeScoringVisualizer->OnRegister();
		}

		const TSharedPtr<FComponentVisualizer> IntenSelectableCylinderScoringVisualizer = MakeShareable(new FIntenSelectableCylinderScoringVisualizer());
		
		if (IntenSelectableCylinderScoringVisualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UIntenSelectableCylinderScoring::StaticClass()->GetFName(), IntenSelectableCylinderScoringVisualizer);
			IntenSelectableCylinderScoringVisualizer->OnRegister();
		}
	}
}

void FRWTHVRToolkitEditorModule::ShutdownModule()
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableLineScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableMultiPointScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableCircleScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableRectangleScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableSphereScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableCubeScoring::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UIntenSelectableCylinderScoring::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE