// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ClusterRepresentationActor.generated.h"


class ARWTHVRPlayerState;
#if PLATFORM_SUPPORTS_CLUSTER
class ADisplayClusterRootActor;
#endif

UCLASS()
class RWTHVRTOOLKIT_API AClusterRepresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AClusterRepresentationActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void AttachDCRAIfRequired(const ARWTHVRPlayerState* OptionalPlayerState = nullptr);

#if PLATFORM_SUPPORTS_CLUSTER
	ADisplayClusterRootActor* GetCachedDCRA() const { return CachedDCRA.Get(); }
#endif

	// Position delta (cm) below which DCRA transform updates are suppressed to kill server idle noise
	UPROPERTY(EditAnywhere, Category = "Cluster|Deadzone")
	float PositionDeadzoneCm = 1.0f;

	// Rotation delta (degrees) below which DCRA rotation updates are suppressed
	UPROPERTY(EditAnywhere, Category = "Cluster|Deadzone")
	float RotationDeadzoneDeg = 0.5f;

private:
	bool bIsAttached = false;

#if PLATFORM_SUPPORTS_CLUSTER
	bool AttachDCRA();
	ADisplayClusterRootActor* SpawnDCRA();

	TWeakObjectPtr<ADisplayClusterRootActor> CachedDCRA;
	FVector LastAppliedDCRALoc = FVector::ZeroVector;
	FRotator LastAppliedDCRARot = FRotator::ZeroRotator;
#endif
};
