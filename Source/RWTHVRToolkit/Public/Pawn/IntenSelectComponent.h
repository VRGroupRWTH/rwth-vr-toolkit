#pragma once

#pragma region /** Includes */
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/WidgetInteractionComponent.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectable.h"
#include "IntenSelectComponent.generated.h"
#pragma endregion

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewComponent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class RWTHVRTOOLKIT_API UIntenSelectComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()


	//	VARIABLES

#pragma region /** INTERNAL VARIABLES */
private:
	float SphereCastRadius;
	float FeedbackCooldown;
	bool IsGrabbing;
	bool IsWidgetInFocus;
	FVector WidgetFocusPoint;
	FVector LastKnownGrabPoint;

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

#pragma endregion

#pragma region /** SETTINGS */
public:
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
#pragma endregion

#pragma region /** REFERENCES */
public:
	UPROPERTY(EditAnywhere, Category = "IntenSelect|References")
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
#pragma endregion

#pragma region /** DEBUG */
public:
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	bool bDrawDebugCone = false;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	bool bDrawForwardRay = true;
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	FRotator DebugConeRotation = FRotator(90, 0, 0);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	FVector DebugConeScale = FVector(1, 1, 1);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	FVector DebugConePosition = FVector(-90, 0, 0);
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Debug")
	float DebugRayTransparency = 1;
#pragma endregion


#pragma region /** Input */
public:
	UPROPERTY(EditAnywhere, Category = "IntenSelect|Input")
	UInputAction* InputClick;

#pragma endregion

#pragma region /** EVENTS */
public:
	UPROPERTY(BlueprintAssignable)
	FOnNewComponent OnNewSelectedEvent;
#pragma endregion


	// FUNCTIONS

#pragma region /** INITIALIZATION */
private:
	/**
	 * \brief Initializes the input bindings for interacting with the component.
	 */
	void InitInputBindings();

	/**
	 * \brief Initializes the debug cone mesh component used for visualization.
	 */
	void InitDebugConeMeshComponent();

	/**
	 * \brief Initializes the spline mesh component used for visualization.
	 */
	void InitSplineMeshComponent();

	/**
	 * \brief Initializes the spline component used for visualization.
	 */
	void InitSplineComponent();

	/**
	 * \brief Initializes the forward ray mesh component used for visualization.
	 */
	void InitForwardRayMeshComponent();

	/**
	 * \brief Initializes the material parameter collection used for visualization.
	 */
	void InitMaterialParamCollection();
#pragma endregion

#pragma region /** SCORING */

private:
	/**
	 * \brief Calculates the radius for sphere casting based on the selection cone angle and maximum selection distance.
	 * \return The calculated sphere cast radius.
	 */
	float CalculateSphereCastRadius() const;

	/**
	 * \brief Retrieves actors from sphere casting to determine potential selections.
	 * \param SphereCastStart The starting point of the sphere cast.
	 * \param OutHits The array to store hit results.
	 * \return True if successful, false otherwise.
	 */
	bool GetActorsFromSphereCast(const FVector& SphereCastStart, TArray<FHitResult>& OutHits) const;

	/**
	 * \brief Checks if a point is within the selection cone.
	 * \param ConeStartPoint The starting point of the selection cone.
	 * \param ConeForward The forward direction of the selection cone.
	 * \param PointToTest The point to test for inclusion in the cone.
	 * \param Angle The angle of the selection cone.
	 * \return True if the point is within the cone, false otherwise.
	 */
	bool CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward, const FVector PointToTest,
						  const float Angle) const;

	/**
	 * \brief Determines the actor with the maximum score for selection.
	 * \param DeltaTime The time elapsed since the last frame.
	 * \return The actor with the maximum score for selection.
	 */
	UIntenSelectable* GetMaxScoreActor(const float DeltaTime);

#pragma endregion


#pragma region /** VISUALS */

private:
	/**
	 * \brief Draws a selection curve from the component to the specified end point.
	 * \param EndPoint The end point of the selection curve.
	 */
	void DrawSelectionCurve(const FVector& EndPoint) const;

	/**
	 * \brief Adds default spline points for creating a spline between start and end points.
	 * \param StartPoint The starting point of the spline.
	 * \param Forward The forward direction of the spline.
	 * \param EndPoint The end point of the spline.
	 */
	void AddSplinePointsDefault(const FVector& StartPoint, const FVector& Forward, const FVector& EndPoint) const;

	/**
	 * \brief Updates the forward ray visualization based on the reference point.
	 * \param ReferencePoint The reference point to update the forward ray to.
	 */
	void UpdateForwardRay(const FVector& ReferencePoint) const;

#pragma endregion


#pragma region /** RAYCASTING */

private:
	/**
	 * \brief Handles the interaction with widgets.
	 */
	void HandleWidgetInteraction();

	/**
	 * \brief Performs a raycast from the start to the end point and returns the first hit result, if any.
	 * \param Start The starting point of the raycast.
	 * \param End The ending point of the raycast.
	 * \return The optional hit result of the raycast.
	 */
	TOptional<FHitResult> RaytraceForFirstHit(const FVector& Start, const FVector& End) const;

#pragma endregion


#pragma region /** INPUT-HANDLING */

private:
	/**
	 * \brief Handles the input event when the fire button is pressed.
	 */
	UFUNCTION(BlueprintCallable)
	void OnFireDown();

	/**
	 * \brief Handles the input event when the fire button is released.
	 */
	UFUNCTION(BlueprintCallable)
	void OnFireUp();

#pragma endregion


#pragma region /** OTHER */

private:
	/**
	 * \brief Handles the cooldown mechanism based on the passed time interval.
	 * \param DeltaTime The time elapsed since the last frame.
	 */
	void HandleCooldown(const float DeltaTime);

	/**
	 * \brief Handles the scenario when no actor is selected.
	 */
	void HandleNoActorSelected();

	/**
	 * \brief Handles the selection of a new actor.
	 * \param NewSelection The new selectable component to be selected.
	 */
	void HandleActorSelected(UIntenSelectable* NewSelection);

	/**
	 * \brief Converts a network quantized vector to a regular FVector.
	 * \param v The network quantized vector to convert.
	 * \return The converted FVector.
	 */
	FVector ConvertNetVector(FVector_NetQuantize v);

#pragma endregion


public:
	/**
	 * \brief Constructor for the UIntenSelectComponent class.
	 */
	UIntenSelectComponent(const FObjectInitializer& ObjectInitializer);

#pragma region /** SELECTION */

	/**
	 * \brief Selects a given selectable component.
	 * \param SelectableComponent The selectable component to be selected.
	 * \param SelectedBy The actor responsible for the selection.
	 */
	void SelectObject(UIntenSelectable* SelectableComponent, AActor* SelectedBy);

	/**
	 * \brief Unselects the currently selected object.
	 */
	void Unselect();

#pragma endregion

	/**
	 * \brief Sets whether the component is active or not.
	 * \param bNewActive The new active state of the component.
	 * \param bReset Whether the activation should happen even if ShouldActivate returns false.
	 */
	virtual void SetActive(bool bNewActive, bool bReset) override;

	/**
	 * \brief Blueprint-native event called when a new object is selected.
	 * \param Selection The newly selected object.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnNewSelected(UIntenSelectable* Selection);

protected:
	/**
	 * \brief Called when the game starts.
	 */
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
};
