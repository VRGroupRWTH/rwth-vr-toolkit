// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClusterSetupComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UClusterSetupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UClusterSetupComponent();

	virtual void InitializeComponent() override;

	/* Add a sync component to all instances of this pawn */
	UFUNCTION(Reliable, NetMulticast)
	void MulticastAddDCSyncComponent();

protected:
	/* Attaches the Cluster representation to the pawn */
	void AttachClusterToPawn();

	UFUNCTION()
	void OnNotifyControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

private:
	UPROPERTY(VisibleAnywhere, Category = "Pawn")
	TObjectPtr<USceneComponent> SyncComponent;
};
