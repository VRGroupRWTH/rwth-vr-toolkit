// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CAVEOverlay/CAVEOverlay.h"

void FCAVEOverlay::Register()
{
	On_Post_World_Initialization_Delegate.BindRaw(this, &FCAVEOverlay::OnSessionStart);
	SessionStartDelegate = FWorldDelegates::OnPostWorldInitialization.Add(On_Post_World_Initialization_Delegate);
}


void FCAVEOverlay::Unregister() const
{
	FWorldDelegates::OnPostWorldInitialization.Remove(SessionStartDelegate);
}

void FCAVEOverlay::OnSessionStart(UWorld* World, const UWorld::InitializationValues)
{
	if (!World->IsGameWorld())
		return;

	const UCAVEOverlaySettings* Settings = GetDefault<UCAVEOverlaySettings>();

	/* Test if we already spawned a CAVEOverlayController */
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, ACAVEOverlayController::StaticClass(), Actors);

	if((Settings->DefaultActivationType == DefaultActivationType_ON
		!= Settings->excludedMaps.ContainsByPredicate(
			[World](const FSoftObjectPath& Map)	{return Map.GetAssetName() == World->GetName();}
		)) && Actors.Num() == 0)
	{
		World->SpawnActor(ACAVEOverlayController::StaticClass());
	}
}

