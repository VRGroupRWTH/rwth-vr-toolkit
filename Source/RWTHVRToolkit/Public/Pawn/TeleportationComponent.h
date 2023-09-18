// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStaticsTypes.h"


#include "TeleportationComponent.generated.h"


UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UTeleportationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTeleportationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bAllowTurning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta=(EditCondition="bAllowTurning"))
	bool bSnapTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning",
		meta=(EditCondition="!bSnapTurn && bAllowTurning"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning",
		meta=(EditCondition="bSnapTurn && bAllowTurning", ClampMin=0, ClampMax=360))
	float SnapTurnAngle = 22.5;

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
	class UInputMappingContext* IMCTeleportLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCTeleportRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopRotation;

	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnStartTeleportTrace(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void UpdateTeleportTrace(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnEndTeleportTrace(const FInputActionValue& Value);


	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);

	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION()
	void StartDesktopRotation();

	UFUNCTION()
	void EndDesktopRotation();

	bool bApplyDesktopRotation = false;


	// Trace Visualization
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BPTeleportVisualizer;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* TeleportTraceSystem;

	UPROPERTY()
	UNiagaraComponent* TeleportTraceComponent;
	
private:
	UPROPERTY()
	UUniversalTrackedComponent* TeleportationHand;

	UPROPERTY()
	UUniversalTrackedComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCMovement;

	void SetupInputActions();

	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	/**
	* Fixes camera rotation in desktop mode.
	*/
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction();

	bool bTeleportTraceActive;
	float TeleportProjectileRadius = 3.6;
	float RotationArrowRadius = 10.0;
	FPredictProjectilePathResult PredictResult;
	bool bValidTeleportLocation = false;
	FVector FinalTeleportLocation;

	UPROPERTY()
	AActor* TeleportVisualizer;
	
};
