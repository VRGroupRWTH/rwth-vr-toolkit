#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"

#include "WalkingPawnMovement.generated.h"

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8
{
	nav_mode_none UMETA(DisplayName = "Navigation Mode None"),
	nav_mode_walk UMETA(DisplayName = "Navigation Mode Walk"),
	nav_mode_fly UMETA(DisplayName = "Navigation Mode Fly")
};

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API UWalkingPawnMovement : public UFloatingPawnMovement
{
	GENERATED_UCLASS_BODY()

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void AddInputVector(FVector WorldVector, bool bForce = false) override;

	void SetCameraComponent(UCameraComponent* NewCameraComponent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WalkingMovement")
	EVRNavigationModes NavigationMode = EVRNavigationModes::nav_mode_fly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WalkingMovement")
	float MaxStepHeight = 40.0f;

private:
	FHitResult CreateLineTrace(FVector Direction, const FVector Start, bool Visibility);
	FHitResult CreateMultiLineTrace(FVector Direction, const FVector Start, float Radius, bool Visibility);
	void SetCapsuleColliderCharacterSizeVR();
	void CheckForPhysWalkingCollision();
	void WalkingModeInput(FVector WorldDirection, bool bForce);
	void MoveByGravityOrStepUp(float DeltaSeconds);
	void ShiftVertically(float DiffernceDistance, float VerticalAcceleration, float DeltaSeconds, int Direction);
	//(direction = Down = -1), (direction = Up = 1)

	UPROPERTY() UCapsuleComponent* CapsuleColliderComponent = nullptr;
	UPROPERTY() UCameraComponent* CameraComponent = nullptr;

	float VerticalSpeed = 0.0f;
	UPROPERTY() float GravityAcceleration = 981.0f;
	UPROPERTY() float UpSteppingAcceleration = 500.0f;
	FVector LastCameraPosition;
	float LastDeltaTime = 0.0f;
};
