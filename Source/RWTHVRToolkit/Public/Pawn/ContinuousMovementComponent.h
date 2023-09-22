// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementExtensionInterface.h"
#include "Pawn/VirtualRealityPawn.h"
#include "Components/ActorComponent.h"
#include "Utility/VirtualRealityUtilities.h"
#include "ContinuousMovementComponent.generated.h"


UENUM(BlueprintType)
enum class EVRSteeringModes : uint8
{
	STEER_GAZE_DIRECTED UMETA(DisplayName = "Gaze Directed (Movement input is applied in head component direction)"),
	STEER_HAND_DIRECTED UMETA(DisplayName = "Hand Direced (Movement input is applied in controller direction)")
};

/**
 * 
 */
UCLASS(Blueprintable)
class RWTHVRTOOLKIT_API UContinuousMovementComponent : public UActorComponent, public IMovementExtensionInterface
{
	GENERATED_BODY()

public:

	UContinuousMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	EVRSteeringModes SteeringMode = EVRSteeringModes::STEER_HAND_DIRECTED;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bMoveWithRightHand = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "VR Movement")
	bool bAllowTurning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta=(EditCondition="bAllowTurning"))
	bool bSnapTurn = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta=(EditCondition="!bSnapTurn && bAllowTurning"))
	float TurnRateFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement|Turning", meta=(EditCondition="bSnapTurn && bAllowTurning",ClampMin=0,ClampMax=360))
	float SnapTurnAngle = 22.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input")
	class UInputMappingContext* IMCMovementRight;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* DesktopRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR Movement|Input|Actions")
	class UInputAction* MoveUp;
	
	/*Movement Input*/
	UFUNCTION(BlueprintCallable)
	void OnBeginMove(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnBeginTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginSnapTurn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void OnBeginUp(const FInputActionValue& Value);
	
	/*Desktop Testing*/
	// the idea is that you have to hold the right mouse button to do rotations
	UFUNCTION()
	void StartDesktopRotation();
	
	UFUNCTION()
	void EndDesktopRotation();
	
	bool bApplyDesktopRotation = false;

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

private:
	
	UPROPERTY()
	UMotionControllerComponent* MovementHand;
	
	UPROPERTY()
	UMotionControllerComponent* RotationHand;

	UPROPERTY()
	class UInputMappingContext* IMCMovement;
	
	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	/**
	* Fixes camera rotation in desktop mode.
	*/
	void SetCameraOffset() const;
	void UpdateRightHandForDesktopInteraction() const;

//~ Begin Replication
	
protected:

	/*
	* For now, replicate in a naive sending every x ms if the transform has changed.
	* This is way overkill, as we should only be sending input. However, I am not yet fully sure how
	* the Unreal Client-Authoritative thingy works and what part simulates e.g. gravity.
	* As this modifies only the tracking origin, latency should not be that much of an issue, so theoretically
	* Server-Authority should work here too, in which case we'd just send the x and y input.
	* Try both ways.
	*/	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Full transform update replication
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	// Rate to update the position to the server, 100htz is default (same as replication rate, should also hit every tick).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Networking", meta = (ClampMin = "0", UIMin = "0"))
	float ControllerNetUpdateRate;

	// Accumulates time until next send
	float ControllerNetUpdateCount;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedTransform, Category = "Networking")
	FVRTransformRep ReplicatedTransform;
	
	void UpdateState(float DeltaTime);

	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		// Modify pawn position - how does this work in movement components?
		// For now, directly apply the transforms:
		auto* Pawn = GetOwner();
		if (Pawn && Pawn->HasValidRootComponent())
			Pawn->SetActorLocationAndRotation(ReplicatedTransform.Position, ReplicatedTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
	void SendControllerTransform_ServerRpc(FVRTransformRep NewTransform);
	
//~ End Replication	
};
