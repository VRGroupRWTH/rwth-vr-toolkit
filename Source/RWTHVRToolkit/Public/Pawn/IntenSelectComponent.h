// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
//#include "SteamVRInputDeviceFunctionLibrary.h"
#include "Components/WidgetInteractionComponent.h"
#include "Interaction/GrabbingBehaviorComponent.h"
#include "Interaction/IntenSelectable.h"
#include "IntenSelectComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewComponent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UIntenSelectComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()


private:
	float SphereCastRadius;
	float FeedbackCooldown;
	
	bool IsGrabbing;
	bool IsWidgetInFocus;
	
	FVector WidgetFocusPoint;

	UPROPERTY()
		UWidgetComponent* LastFocusedWidget;
	UPROPERTY()
		TMap<UIntenSelectable*, float> ScoreMap;
	UPROPERTY()
		TMap<UIntenSelectable*, FHitResult> ContactPointMap;
	UPROPERTY()
		UIntenSelectable* CurrentSelection;
	UPROPERTY()
		UIntenSelectable* LastKnownSelection;
	UPROPERTY()
		FVector LastKnownGrabPoint;
	UPROPERTY()
		UGrabbingBehaviorComponent* GrabbedActorBehaviourComponent;
	UPROPERTY()
		UStaticMeshComponent* DebugConeMeshComponent;
	UPROPERTY()
		UStaticMeshComponent* ForwardRayMeshComponent;
	UPROPERTY()
		USplineComponent* SplineComponent;
	UPROPERTY()
		USplineMeshComponent* SplineMeshComponent;
	UPROPERTY()
		UMaterialParameterCollectionInstance* ParameterCollectionInstance;
	UPROPERTY()
		UStaticMesh* DebugConeMesh;
	UPROPERTY()
		UMaterialInterface* DebugConeMaterial;
	UPROPERTY()
		UStaticMesh* ForwardRayMesh;

public:

	//		SETTINGS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		bool SetActiveOnStart = true;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float MaxSelectionDistance = 5000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float SelectionConeAngle = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float SplineCurvatureStrength = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float ConeBackwardShiftDistance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float MaxClickStickAngle = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|Settings")
		float ForwardRayWidth = 0.01;

	//		REFERENCES
	
	UPROPERTY(EditAnywhere, Category= "IntenSelect|References")
		UStaticMesh* SplineMesh;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		UMaterialInterface* SplineMaterial;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		UMaterialInterface* ForwardRayMaterial;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		UHapticFeedbackEffect_Base* SelectionFeedbackHaptic;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		USoundBase* OnSelectSound;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		UMaterialParameterCollection* MaterialParamCollection;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
		UCurveFloat* ForwardRayTransparencyCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IntenSelect|References")
		TArray<AActor*> ActorsToIgnore;

	//		DEBUG VARIABLES

	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		FRotator DebugConeRotation = FRotator(90, 0, 0);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		FVector DebugConeScale = FVector(1,1,1);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		FVector DebugConePosition = FVector(-90, 0, 0);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		bool bDrawDebugCone = false;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		bool bDrawForwardRay = true;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
		float DebugRayTransparency = 1;
	
	//		OTHER

	UPROPERTY(BlueprintAssignable)
		FOnNewComponent OnNewSelectedEvent;

private:

	//	INITIALIZATION
	void InitInputBindings();
	void InitDebugConeMeshComponent();
	void InitSplineMeshComponent();
	void InitSplineComponent();
	void InitForwardRayMeshComponent();
	void InitMaterialParamCollection();

	// SCORING
	float CalculateSphereCastRadius() const;
	bool GetActorsFromSphereCast(const FVector& SphereCastStart, TArray<FHitResult>& OutHits) const;
	bool CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward, const FVector PointToTest, const float Angle) const;
	UIntenSelectable* GetMaxScoreActor(const float DeltaTime);

	//	VISUALS
	void DrawSelectionCurve(const FVector& EndPoint) const;
	void AddSplinePointsDefault(const FVector& StartPoint, const FVector& Forward, const FVector& EndPoint) const;
	void UpdateForwardRay(const FVector& ReferencePoint) const;

	//	RAYCASTING
	void HandleWidgetInteraction();
	TOptional<FHitResult> RaytraceForFirstHit(const FVector& Start, const FVector& End) const;

	//	INPUT-HANDLING
	UFUNCTION(BlueprintCallable)
	void OnFireDown();
	UFUNCTION(BlueprintCallable)
	void OnFireUp();

	//	OTHER
	void HandleCooldown(const float DeltaTime);
	void HandleGrabbing(const float DeltaTime) const;
	void HandleNoActorSelected();
	void HandleActorSelected(UIntenSelectable* NewSelection);
	FVector ConvertNetVector(FVector_NetQuantize v);

public:	
	UIntenSelectComponent(const FObjectInitializer& ObjectInitializer);

	//	SELECTION
	void SelectObject(UGrabbingBehaviorComponent* GrabbingBehaviourComponent, UIntenSelectable* SelectableComponent, AActor* SelectedBy);
	void Unselect();

	virtual void SetActive(bool bNewActive, bool bReset) override;
	
	UFUNCTION(BlueprintNativeEvent)
		void OnNewSelected(UIntenSelectable* Selection);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
