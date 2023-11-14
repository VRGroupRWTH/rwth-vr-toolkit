// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/Navigation/TeleportationComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationSystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Utility/VirtualRealityUtilities.h"
#include "MotionControllerComponent.h"


void UTeleportationComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInput(PlayerInputComponent);

	if (!VRPawn || !VRPawn->HasLocalNetOwner() || !InputSubsystem)
	{
		return;
	}

	TeleportTraceComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		TeleportTraceSystem,
		VRPawn->GetActorLocation(),
		FRotator(0),
		FVector(1),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true
	);

	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.Name = "TeleportVisualizer";

	if (BPTeleportVisualizer)
	{
		TeleportVisualizer = GetWorld()->SpawnActor<AActor>(BPTeleportVisualizer, VRPawn->GetActorLocation(),
		                                                    VRPawn->GetActorRotation(), SpawnParameters);
	}
	TeleportTraceComponent->SetVisibility(false);
	TeleportVisualizer->SetActorHiddenInGame(true);

	// simple way of changing the handedness
	if (bMoveWithRightHand)
	{
		TeleportationHand = VRPawn->RightHand;
		RotationHand = VRPawn->LeftHand;
		IMCMovement = IMCTeleportRight;
	}
	else
	{
		TeleportationHand = VRPawn->LeftHand;
		RotationHand = VRPawn->RightHand;
		IMCMovement = IMCTeleportLeft;
	}

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement, 0);

	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EI)
	{
		UE_LOG(Toolkit, Error, TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}

	// teleporting
	EI->BindAction(Move, ETriggerEvent::Started, this, &UTeleportationComponent::OnStartTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Triggered, this, &UTeleportationComponent::UpdateTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Completed, this, &UTeleportationComponent::OnEndTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Canceled, this, &UTeleportationComponent::OnEndTeleportTrace);

	// turning is defined in MovementComponentBase
}

// On button press -> show teleport trace
void UTeleportationComponent::OnStartTeleportTrace(const FInputActionValue& Value)
{
	// Start Trace
	bTeleportTraceActive = true;
	TeleportTraceComponent->SetVisibility(true);
	TeleportVisualizer->SetActorHiddenInGame(false);
}

// called while button is pressed (Triggered)
void UTeleportationComponent::UpdateTeleportTrace(const FInputActionValue& Value)
{
	// Update the teleport trace
	const FVector StartPosition = TeleportationHand->GetComponentLocation();
	const FVector ForwardVector = TeleportationHand->GetForwardVector();

	TArray<AActor> ActorsToIgnore;

	FPredictProjectilePathParams PredictParams = FPredictProjectilePathParams
	(
		TeleportProjectileRadius,
		StartPosition,
		TeleportLaunchSpeed * ForwardVector,
		5.0,
		ECC_WorldStatic
	);

	PredictParams.ActorsToIgnore.Add(VRPawn);
	PredictParams.ActorsToIgnore.Add(TeleportVisualizer);

	UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);

	const FVector HitLocation = PredictResult.HitResult.Location;
	const bool bValidHit = PredictResult.HitResult.IsValidBlockingHit();
	// check if this is a valid location to move to

	FVector OutLocation;
	const bool bValidProjection = IsValidTeleportLocation(PredictResult.HitResult, OutLocation);

	if (bUseNavMesh)
	{
		FinalTeleportLocation = OutLocation;
		if (bValidTeleportLocation != bValidProjection)
		{
			bValidTeleportLocation = bValidProjection;
			TeleportVisualizer->SetActorHiddenInGame(!bValidTeleportLocation);
		}
	}
	else
	{
		if (bValidHit)
		{
			FinalTeleportLocation = HitLocation;
			TeleportVisualizer->SetActorHiddenInGame(false);
			// update location
			TeleportVisualizer->SetActorLocation(FinalTeleportLocation);
		}
	}

	TArray<FVector> PathPoints;
	PathPoints.Add(StartPosition);
	for (FPredictProjectilePathPointData PData : PredictResult.PathData)
	{
		PathPoints.Add(PData.Location);
	}
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TeleportTraceComponent, FName("User.PointArray"),
	                                                                 PathPoints);
}

bool UTeleportationComponent::IsValidTeleportLocation(const FHitResult& Hit, FVector& ProjectedLocation) const
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const FNavAgentProperties& AgentProps = FNavAgentProperties(15, 160);
	FNavLocation ProjectedNavLocation;
	const bool bProjectPoint = (NavSys && NavSys->ProjectPointToNavigation(
		Hit.Location, ProjectedNavLocation, INVALID_NAVEXTENT, &AgentProps));
	ProjectedLocation = ProjectedNavLocation.Location;
	return bProjectPoint /*&& Hit.IsValidBlockingHit()*/;
}

// On button release -> remove trace and teleport user to location
void UTeleportationComponent::OnEndTeleportTrace(const FInputActionValue& Value)
{
	if (!VRPawn)
		return;
	// End Teleport Trace
	bTeleportTraceActive = false;
	TeleportTraceComponent->SetVisibility(false);
	TeleportVisualizer->SetActorHiddenInGame(true);

	bValidTeleportLocation = false;
	VRPawn->TeleportTo(FinalTeleportLocation, VRPawn->GetActorRotation());
}
