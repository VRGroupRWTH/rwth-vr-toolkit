#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"

#include "VRPawnMovement.generated.h"

/*
 * This Movement component is needed since in VR not only the pawn itself (UpdatedComponent) is moved but also the
 * user herself can walk and thereby move the CameraComponent, which can also lead to collisions or e.g. going up steps 
 *
 * The four modes are:
 * None: No controller movement is applied and no corrections regarding steps or collisions with walls are done
 * Ghost: The same as above but now the Inputs can be used for unconstrained flying (also through objects)
 * Fly: The user can fly but not through walls etc. When the user walks against a wall the scene is moved with her to avoid walking through
 *      The user can also walk up stairs with a maximum step height of MaxStepHeight
 * Walk: Additionally to Fly now gravity keeps the user on the floor
 */

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8
{
	NAV_NONE UMETA(DisplayName = "None (no controller movement)"),
	NAV_GHOST UMETA(DisplayName = "Ghost (flying, also through walls)"),
	NAV_FLY UMETA(DisplayName = "Fly (prohibiting collisions)"),
	NAV_WALK UMETA(DisplayName = "Walk (gravity and prohibiting collisions)")
};

UCLASS()
class RWTHVRTOOLKIT_API UVRPawnMovement : public UFloatingPawnMovement
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
	void CheckAndRevertCollisionSinceLastTick();

	void MoveOutOfNewDynamicCollisions();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetHeadComponent(USceneComponent* NewHeadComponent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	EVRNavigationModes NavigationMode = EVRNavigationModes::NAV_WALK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta = (ClampMin="0.0"))
	float MaxStepHeight = 40.0f;

	// if the height that the pawn would fall (in walking mode) is higher
	// it is not falling, set to <0.0f if you want to fall infinitely
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float MaxFallingDepth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta = (ClampMax="0.0"))
	float GravityAcceleration = -981.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta = (ClampMin="0.0"))
	float UpSteppingAcceleration = 981.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement", meta = (ClampMin="0.0"))
	float CapsuleRadius = 40.0f;

private:
	//check for
	FHitResult CreateCapsuleTrace(const FVector& Start, const FVector& End, bool DrawDebug = false) const;
	FVector GetOverlapResolveDirection();
	void SetCapsuleColliderToUserSize() const;
	void CheckForPhysWalkingCollision();
	FVector GetCollisionSafeVirtualSteeringVec(FVector InputVector, float DeltaTime);
	void MoveByGravityOrStepUp(float DeltaSeconds);
	void ShiftVertically(float Distance, float VerticalAcceleration, float DeltaSeconds);

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CapsuleColliderComponent = nullptr;
	UPROPERTY()
	USceneComponent* HeadComponent = nullptr;

	float VerticalSpeed = 0.0f;
	FVector LastCollisionFreeCapsulePosition;
	FVector LastSteeringCollisionVector;

	//just stored for performance gains;
	UPROPERTY()
	TArray<AActor*> ActorsToIgnore;
};
