// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Kismet/KismetSystemLibrary.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetInteractionComponent.h"
#include "RaycastSelectComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewComponentRaycast);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API URaycastSelectComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
		UStaticMeshComponent* ForwardRayMeshComponent;
	UPROPERTY()
		UStaticMesh* ForwardRayMesh;
	UPROPERTY()
		bool IsGrabbing = false;
	UPROPERTY()
		URaycastSelectable* CurrentSelection;
	UPROPERTY()
		FVector CurrentSelectionPoint;
	UPROPERTY()
		URaycastSelectable* LastKnownGrab;
	UPROPERTY()
		FVector LastKnownGrabPoint;

public:

	//		SETTINGS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Settings")
		bool SetActiveOnStart = true;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Settings")
		float MaxSelectionDistance = 10000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Settings")
		float MaxClickStickAngle = 20;

	//		REFERENCES
	
	UPROPERTY(EditAnywhere, Category = "Raycast|References")
		UMaterialInterface* ForwardRayMaterial;
	UPROPERTY(EditAnywhere, Category = "Raycast|References")
		UHapticFeedbackEffect_Base* SelectionFeedbackHaptic;
	UPROPERTY(EditAnywhere, Category = "Raycast|References")
		USoundBase* OnSelectSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|References")
		TArray<AActor*> ActorsToIgnore;

	//		DEBUG VARIABLES

	UPROPERTY(EditAnywhere, Category = "Raycast|Debug")
		bool bDrawForwardRay = true;
	UPROPERTY(EditAnywhere, Category = "Raycast|Debug")
		bool bDrawDebugRay = false;
	UPROPERTY(EditAnywhere, Category = "Raycast|Debug")
		bool bShowHitsOnScreen = false;
	
	//		OTHER

	UPROPERTY(BlueprintAssignable)
		FOnNewComponentRaycast OnNewSelectedEvent;

private:

	//	INITIALIZATION
	void InitInputBindings();
	void InitForwardRayMeshComponent();

	// SCORING
	static bool CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward, const FVector PointToTest, const float ConeAngle);

	TOptional<FHitResult> RaytraceForFirstHit(const FVector& Start, const FVector& End) const;

	//	INPUT-HANDLING
	UFUNCTION(BlueprintCallable)
	void OnFireDown();
	UFUNCTION(BlueprintCallable)
	void OnFireUp();

public:	
	URaycastSelectComponent(const FObjectInitializer& ObjectInitializer);

	virtual void SetActive(bool bNewActive, bool bReset) override;

	UFUNCTION(BlueprintNativeEvent)
		void OnNewSelected(UIntenSelectable* Selection);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
