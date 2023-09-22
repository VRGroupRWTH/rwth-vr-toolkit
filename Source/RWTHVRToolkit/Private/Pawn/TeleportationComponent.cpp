// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/TeleportationComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationSystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VRPawnInputConfig.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Utility/VirtualRealityUtilities.h"
#include "MotionControllerComponent.h"

// Sets default values for this component's properties
UTeleportationComponent::UTeleportationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTeleportationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	TeleportTraceComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		TeleportTraceSystem,
		GetOwner()->GetActorLocation(),
		FRotator(0),
		FVector(1),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true
	);

	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.Name = "TeleportVisualizer";
	if(BPTeleportVisualizer)
	{
		TeleportVisualizer = GetWorld()->SpawnActor<AActor>(BPTeleportVisualizer,GetOwner()->GetActorLocation(),GetOwner()->GetActorRotation(),SpawnParameters);
	}
	TeleportTraceComponent->SetVisibility(false);
	TeleportVisualizer->SetActorHiddenInGame(true);

	
	// ...
}

// Called every frame
void UTeleportationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTeleportationComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	VRPawn = Cast<AVirtualRealityPawn>(GetOwner());

	// simple way of changing the handedness
	if(bMoveWithRightHand)
	{
		TeleportationHand = VRPawn->RightHand;
		RotationHand = VRPawn->LeftHand;
		IMCMovement = IMCTeleportRight;
	} else
	{
		TeleportationHand = VRPawn->LeftHand;
		RotationHand = VRPawn->RightHand;
		IMCMovement = IMCTeleportLeft;
	}
	
	
	auto* InputSubsystem = GetEnhancedInputLocalPlayerSubsystem(VRPawn);
	if(!InputSubsystem)
	{
		UE_LOG(Toolkit,Error,TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if(!EI)
	{
		UE_LOG(Toolkit,Error,TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}
	
	// walking
	EI->BindAction(Move, ETriggerEvent::Started, this, &UTeleportationComponent::OnStartTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Triggered, this, &UTeleportationComponent::UpdateTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Completed, this, &UTeleportationComponent::OnEndTeleportTrace);
	EI->BindAction(Move, ETriggerEvent::Canceled, this, &UTeleportationComponent::OnEndTeleportTrace);

	// turning
	if(bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(Turn, ETriggerEvent::Started, this, &UTeleportationComponent::OnBeginSnapTurn);
	} else
	{
		EI->BindAction(Turn, ETriggerEvent::Triggered, this, &UTeleportationComponent::OnBeginTurn);
	}
	
	// bind functions for desktop rotations only on holding down right mouse
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
		if (PC)
		{
			PC->bShowMouseCursor = true; 
			PC->bEnableClickEvents = true; 
			PC->bEnableMouseOverEvents = true;
		} else
		{
			UE_LOG(LogTemp,Error,TEXT("PC Player Controller is invalid"));
		}
		EI->BindAction(DesktopRotation, ETriggerEvent::Started, this, &UTeleportationComponent::StartDesktopRotation);
		EI->BindAction(DesktopRotation, ETriggerEvent::Completed, this, &UTeleportationComponent::EndDesktopRotation);
	}
}

void UTeleportationComponent::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void UTeleportationComponent::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
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
	FVector StartPosition = TeleportationHand->GetComponentLocation();
	FVector ForwardVector = TeleportationHand->GetForwardVector();

	TArray<AActor> ActorsToIgnore;
	
	FPredictProjectilePathParams PredictParams = FPredictProjectilePathParams
	(
		TeleportProjectileRadius,
		StartPosition,
		TeleportLaunchSpeed * ForwardVector,
		5.0,
		ECC_WorldStatic
	);

	PredictParams.ActorsToIgnore.Add(GetOwner());
	PredictParams.ActorsToIgnore.Add(TeleportVisualizer);
	
	UGameplayStatics::PredictProjectilePath(GetWorld(),PredictParams,PredictResult);
	
	FVector HitLocation = PredictResult.HitResult.Location;
	bool bValidHit = PredictResult.HitResult.IsValidBlockingHit();
	// check if this is a valid location to move to
	FNavLocation OutLocation;

	FNavAgentProperties AgentProperties = FNavAgentProperties(15, 160);
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	// TODO: does not give valid location
	const bool bValidProjection = NavSystem->ProjectPointToNavigation(HitLocation,OutLocation,FVector(1,1,1), &AgentProperties);
	
	if(bUseNavMesh)
	{
		FinalTeleportLocation = OutLocation.Location;
		if(bValidTeleportLocation != bValidProjection)
		{
			bValidTeleportLocation = bValidProjection;
			TeleportVisualizer->SetActorHiddenInGame(!bValidTeleportLocation);
		}
	} else
	{
		if(bValidHit)
		{
			FinalTeleportLocation = HitLocation;
			TeleportVisualizer->SetActorHiddenInGame(false);
			// update location
			TeleportVisualizer->SetActorLocation(FinalTeleportLocation);
		} 
	}


	TArray<FVector> PathPoints;
	PathPoints.Add(StartPosition);
	for(FPredictProjectilePathPointData PData : PredictResult.PathData)
	{
		PathPoints.Add(PData.Location);
	}
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TeleportTraceComponent,FName("User.PointArray"),PathPoints);
	
}

// On button release -> remove trace and teleport user to location
void UTeleportationComponent::OnEndTeleportTrace(const FInputActionValue& Value)
{

	// End Teleport Trace
	bTeleportTraceActive = false;
	TeleportTraceComponent->SetVisibility(false);
	TeleportVisualizer->SetActorHiddenInGame(true);
	
	bValidTeleportLocation = false;
	GetOwner()->TeleportTo(FinalTeleportLocation,GetOwner()->GetActorRotation());
	
}

void UTeleportationComponent::OnBeginTurn(const FInputActionValue& Value)
{
	if(UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation) return;
	if (VRPawn->Controller != nullptr)
	{
		const FVector2D TurnValue = Value.Get<FVector2D>();
 
		if (TurnValue.X != 0.f)
		{
			VRPawn->AddControllerYawInput(TurnRateFactor * TurnValue.X);
			if (UVirtualRealityUtilities::IsDesktopMode())
			{
				UpdateRightHandForDesktopInteraction();
			}
		}
 
		if (TurnValue.Y != 0.f)
		{
			if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
			{
				VRPawn->AddControllerPitchInput(TurnRateFactor * -TurnValue.Y);
				SetCameraOffset();
			}
		}
	}
}

void UTeleportationComponent::OnBeginSnapTurn(const FInputActionValue& Value)
{
	const FVector2D TurnValue = Value.Get<FVector2D>();
	if (TurnValue.X > 0.f)
	{
		VRPawn->AddControllerYawInput(SnapTurnAngle);
	} else if (TurnValue.X < 0.f)
	{
		VRPawn->AddControllerYawInput(-SnapTurnAngle);
	}
}

void UTeleportationComponent::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	VRPawn->GetActorEyesViewPoint(Location, Rotation);
	VRPawn->HeadCameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}

void UTeleportationComponent::UpdateRightHandForDesktopInteraction()
{
	APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		VRPawn->RightHand->SetWorldRotation(HandOrientation);
	}
}

