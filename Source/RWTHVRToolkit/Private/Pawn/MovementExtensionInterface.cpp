// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/MovementExtensionInterface.h"

UEnhancedInputLocalPlayerSubsystem* IMovementExtensionInterface::GetEnhancedInputLocalPlayerSubsystem(APawn* Pawn) const
{
	const APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	const ULocalPlayer* LP = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	
	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();	
}
