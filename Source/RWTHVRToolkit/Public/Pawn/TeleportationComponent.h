// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Pawn/MovementComponentBase.h"

#include "TeleportationComponent.generated.h"

UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UTeleportationComponent : public UMovementComponentBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = true;

	/**
	 * Whether the hit location of the teleport trace should be projected onto the navigation mesh
	 * TODO: does currently not work, so leave it at false
	 */
	UPROPERTY(VisibleAnywhere, Category = "VR Movement|Teleport")
	bool bUseNavMesh = false;

	/**
	 * Speed at which the projectile shoots out from the controller to get the teleport location
	 * Higher values = larger teleportation range
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Teleport")
	float TeleportLaunchSpeed = 800;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	UInputMappingContext* IMCTeleportLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	UInputMappingContext* IMCTeleportRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	UInputAction* Move;

	// Trace Visualization
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BPTeleportVisualizer;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* TeleportTraceSystem;

	UPROPERTY()
	UNiagaraComponent* TeleportTraceComponent;

	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnStartTeleportTrace(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void UpdateTeleportTrace(const FInputActionValue& Value);
	bool IsValidTeleportLocation(const FHitResult& Hit, FVector& ProjectedLocation) const;

	UFUNCTION(BlueprintCallable)
	void OnEndTeleportTrace(const FInputActionValue& Value);

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	UMotionControllerComponent* TeleportationHand;

	UPROPERTY()
	UMotionControllerComponent* RotationHand;

	UPROPERTY()
	UInputMappingContext* IMCMovement;

	bool bTeleportTraceActive;
	float TeleportProjectileRadius = 3.6;
	float RotationArrowRadius = 10.0;
	FPredictProjectilePathResult PredictResult;
	bool bValidTeleportLocation = false;
	FVector FinalTeleportLocation;

	UPROPERTY()
	AActor* TeleportVisualizer;
};
