#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Components/DisplayClusterSceneComponent.h"

#include "GameFramework/Actor.h"
#include "CalibratioActor.generated.h"

UCLASS()
class CALIBRATIO_API ACalibratioActor : public AActor
{
	GENERATED_BODY()

public:
	ACalibratioActor();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere) UMaterialInterface* BaseMaterial;
	UPROPERTY(BlueprintReadWrite) float Threshold = FMath::DegreesToRadians(1.7f);
	UPROPERTY(BlueprintReadWrite) float ResetTime = 0.5f;
	UFUNCTION(Blueprintcallable) void ClusterDespawn();

private:
	DECLARE_DELEGATE_OneParam(FThresholdDelegate, float);
	FOnClusterEventJsonListener ClusterEventListenerDelegate;

	void CalibrationRun(float Angle);
	bool IsTrackerCurrentlyVisible();
	void LocalSetThreshold(float NewValue);
	void LocalArmAndSetCalibration(float NewMin, float NewMax);
	void LocalDespawn();
	bool IsDeviceMoving(float Angle);
	void LocalReset();

	// Handling Cluster events
	UFUNCTION() void ClusterReset();
	UFUNCTION() void ClusterIncreaseThreshold();
	UFUNCTION() void ClusterDecreaseThreshold();
	void ClusterChangeThreshold(float Value);
	void ClusterArmAndSetCalibration(float MinAngle, float MaxAngle);
	void HandleClusterEvent(const FDisplayClusterClusterEventJson& Event);

	float MinRotation{0};
	float MaxRotation{0};
	int CurrentCalibrationRuns{0};
	int MaxCalibrationRuns{60};
	FTimerHandle ResetTimerHandle;
	TArray<float> LastRotations;

	UPROPERTY() UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	FDateTime LastTimeVisible = FDateTime::MinValue();
	FVector LastVisiblePosition = FVector(NAN, NAN, NAN);
	bool FirstPositionSet = false;
	uint32 AcceptedAbscenceTime = 500u; // in Milliseconds
	UPROPERTY() USceneComponent* TrackedClusterComponent = nullptr;

	//Overlay
	TSubclassOf<class UCalibratioOverlay> Overlay_Class;
	UPROPERTY() UCalibratioOverlay* Overlay;
};
