#include "Pawn/IntenSelectComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/MessageDialog.h"
#include "Templates/Tuple.h"


//				INITIALIZATION

// Sets default values for this component's properties
UIntenSelectComponent::UIntenSelectComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bShowDebug = false; // otherwise the WidgetInteractionComponent debug vis is shown
	InteractionSource = EWidgetInteractionSource::Custom; // can also be kept at default (World), this way, however, we
														  // efficiently reuse the line traces
}

// Called when the game starts
void UIntenSelectComponent::BeginPlay()
{
	Super::BeginPlay();

	InitSplineComponent();
	InitSplineMeshComponent();
	InitForwardRayMeshComponent();
	InitDebugConeMeshComponent();
	InitInputBindings();
	InitMaterialParamCollection();

	// Calculate sphere cast radius
	SphereCastRadius = CalculateSphereCastRadius();

	// Set interaction distance to maximum selection distance
	InteractionDistance = MaxSelectionDistance;

	// Set the component active based on the SetActiveOnStart flag
	SetActive(SetActiveOnStart, false);
}

void UIntenSelectComponent::InitInputBindings()
{
	// Get the player controller
	const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Get the local player subsystem for enhanced input
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	// Get the player input component
	UInputComponent* PlayerInputComponent = PC->InputComponent;
	UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	// Check if the enhanced input component is valid
	if (!PEI)
	{
		// Display an error message and quit the game if the enhanced input component is not found
		const FString Message = "Could not get PlayerInputComponent for IntenSelect Input Assignment!";
#if WITH_EDITOR
		const FText Title = FText::FromString(FString("ERROR"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), Title);
#endif
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
		return;
	}

	// Bind the actions for input events
	PEI->BindAction(InputClick, ETriggerEvent::Started, this, &UIntenSelectComponent::OnFireDown);
	PEI->BindAction(InputClick, ETriggerEvent::Completed, this, &UIntenSelectComponent::OnFireUp);
}

void UIntenSelectComponent::InitSplineComponent()
{
	// Create a new spline component
	SplineComponent = NewObject<USplineComponent>(this, TEXT("SplineComponent"));

	// Check if the spline component was successfully created
	if (SplineComponent)
	{
		// Setup attachment and mobility of the spline component
		SplineComponent->SetupAttachment(this);
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->RegisterComponent();
		SplineComponent->CreationMethod = EComponentCreationMethod::Instance;
	}
	else
	{
		// Display an error message if the spline component creation fails
		const FString Message = "Error while spawning SplineComponent!";
#if WITH_EDITOR
		const FText Title = FText::FromString(FString("ERROR"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), Title);
#endif
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
	}
}

void UIntenSelectComponent::InitSplineMeshComponent()
{
	// Create a new spline mesh component
	SplineMeshComponent =
		NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), TEXT("SplineMeshComponent"));

	// Check if the spline mesh component was successfully created
	if (SplineMeshComponent)
	{
		// Setup attachment and mobility of the spline mesh component
		SplineMeshComponent->SetupAttachment(this);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->RegisterComponent();
		SplineMeshComponent->CreationMethod = EComponentCreationMethod::Instance;

		// Set the static mesh if available
		if (SplineMesh)
		{
			SplineMeshComponent->SetStaticMesh(SplineMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineMesh not set!"));
		}

		// Set the material if available
		if (SplineMaterial)
		{
			SplineMeshComponent->SetMaterial(0, SplineMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineMesh material not set! Using default material instead."));
		}

		// Set the forward axis and shadow casting properties
		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::Z);
		SplineMeshComponent->CastShadow = false;
	}
	else
	{
		// Display an error message if the spline mesh component creation fails
		UE_LOG(LogTemp, Error, TEXT("Error while spawning SplineMeshComponent!"))
	}
}


void UIntenSelectComponent::InitForwardRayMeshComponent()
{
	// Create a new static mesh component for the forward ray
	ForwardRayMeshComponent =
		NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("ForwardRay"));

	// Check if the forward ray mesh component was successfully created
	if (ForwardRayMeshComponent)
	{
		// Setup attachment and mobility of the forward ray mesh component
		ForwardRayMeshComponent->SetupAttachment(this);
		ForwardRayMeshComponent->SetMobility(EComponentMobility::Movable);
		ForwardRayMeshComponent->RegisterComponent();
		ForwardRayMeshComponent->CreationMethod = EComponentCreationMethod::Instance;

		// Configure shadow casting and collision properties
		ForwardRayMeshComponent->SetCastShadow(false);
		ForwardRayMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set relative scale and location based on max selection distance
		const float MeshLength = MaxSelectionDistance > 1000 ? 1000 : MaxSelectionDistance;
		ForwardRayMeshComponent->SetRelativeScale3D(FVector(MeshLength, ForwardRayWidth, ForwardRayWidth));
		ForwardRayMeshComponent->SetRelativeLocation(FVector(MeshLength * 50, 0, 0));

		// Set the static mesh for the forward ray component if available
		if (ForwardRayMesh)
		{
			ForwardRayMeshComponent->SetStaticMesh(ForwardRayMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Mesh for RayComponent not set!"));
		}

		// Create dynamic material instance for the forward ray mesh component
		UMaterialInstanceDynamic* DynamicMaterial =
			UMaterialInstanceDynamic::Create(ForwardRayMaterial, ForwardRayMeshComponent);
		this->ForwardRayMeshComponent->SetMaterial(0, DynamicMaterial);

		// Set visibility based on draw forward ray flag
		ForwardRayMeshComponent->SetHiddenInGame(!bDrawForwardRay);
	}
	else
	{
		// Display an error message if the forward ray mesh component creation fails
		UE_LOG(LogTemp, Error, TEXT("Error while spawning ForwardRayMesh component!"));
	}
}


void UIntenSelectComponent::InitMaterialParamCollection()
{
	// Check if the material parameter collection is set
	if (MaterialParamCollection)
	{
		// Get the parameter collection instance from the world
		this->ParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialParamCollection);
		if (this->ParameterCollectionInstance)
		{
			// Set the scalar parameter value for transparency
			this->ParameterCollectionInstance->SetScalarParameterValue("Transparency", DebugRayTransparency);
		}
		else
		{
			// Display a warning if the parameter collection instance is not found
			UE_LOG(LogTemp, Warning,
				   TEXT("MaterialParameterCollection required for rendering of IntenSelect could not be found!"))
		}
	}
	else
	{
		// Display a warning if the material parameter collection is not set
		UE_LOG(LogTemp, Warning, TEXT("MaterialParameterCollection required for InteSelect visualization is not set!"));
	}
}

void UIntenSelectComponent::InitDebugConeMeshComponent()
{
	// Create a new static mesh component for the debug cone
	DebugConeMeshComponent =
		NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("DebugCone"));

	// Check if the debug cone mesh component was successfully created
	if (DebugConeMeshComponent)
	{
		// Setup attachment and mobility of the debug cone mesh component
		DebugConeMeshComponent->SetupAttachment(this);
		DebugConeMeshComponent->SetMobility(EComponentMobility::Movable);
		DebugConeMeshComponent->RegisterComponent();
		DebugConeMeshComponent->CreationMethod = EComponentCreationMethod::Instance;

		// Calculate transform for the cone based on selection cone angle and distance
		FTransform ConeTransform = DebugConeMeshComponent->GetRelativeTransform();
		const float ConeScale = MaxSelectionDistance / 50 * FMath::Tan(FMath::DegreesToRadians(SelectionConeAngle));
		ConeTransform.SetScale3D(FVector(ConeScale, ConeScale, MaxSelectionDistance / 100));

		// Set relative transform and location for the debug cone
		DebugConeMeshComponent->SetRelativeTransform(ConeTransform);
		DebugConeMeshComponent->SetRelativeLocation(FVector(MaxSelectionDistance - ConeBackwardShiftDistance, 0, 0),
													false);
		DebugConeMeshComponent->SetRelativeRotation(DebugConeRotation, false);
		DebugConeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set the static mesh for the debug cone component if available
		if (DebugConeMesh)
		{
			DebugConeMeshComponent->SetStaticMesh(DebugConeMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DebugCone mesh not set!"))
		}

		// Set the material for the debug cone component if available
		if (DebugConeMaterial)
		{
			DebugConeMeshComponent->SetMaterial(0, DebugConeMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DebugCone material not set! Using default material instead."))
		}

		// Set visibility based on draw debug cone flag
		DebugConeMeshComponent->SetVisibility(bDrawDebugCone);
	}
	else
	{
		// Display an error message if the debug cone mesh component creation fails
		UE_LOG(LogTemp, Error, TEXT("Error while spawning DebugCone component!"))
	}
}


//				SCORING FUNCTIONS

float UIntenSelectComponent::CalculateSphereCastRadius() const
{
	// Calculate sphere cast radius based on selection cone angle and max selection distance
	return FMath::Tan(FMath::DegreesToRadians(SelectionConeAngle)) * MaxSelectionDistance;
}


bool UIntenSelectComponent::CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward,
											 const FVector PointToTest, const float Angle) const
{
	// Shift the start origin point of the cone backward
	const FVector ShiftedStartOriginPoint = ConeStartPoint - (ConeForward * ConeBackwardShiftDistance);

	// Calculate the direction to the test point
	const FVector DirectionToTestPoint = (PointToTest - ShiftedStartOriginPoint).GetSafeNormal();

	// Calculate the angle to the test point
	const float AngleToTestPoint =
		FMath::RadiansToDegrees(FMath::Acos((FVector::DotProduct(ConeForward, DirectionToTestPoint))));

	// Check if the angle to the test point is within the specified cone angle
	return AngleToTestPoint <= Angle;
}


void UIntenSelectComponent::OnNewSelected_Implementation(UIntenSelectable* Selection)
{
	// Set the current selection
	CurrentSelection = Selection;

	// Play sound feedback if cooldown allows
	if (FeedbackCooldown == 0)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), OnSelectSound);
		FeedbackCooldown = 0.1;
	}
}


bool UIntenSelectComponent::GetActorsFromSphereCast(const FVector& SphereCastStart, TArray<FHitResult>& OutHits) const
{
	// Calculate start and end positions for the sphere cast
	const FVector StartPos =
		SphereCastStart + (GetComponentTransform().GetRotation().GetForwardVector() * SphereCastRadius);
	const FVector EndPos = StartPos + (GetComponentTransform().GetRotation().GetForwardVector() * MaxSelectionDistance);

	// Set up collision query parameters
	const FCollisionQueryParams Params = FCollisionQueryParams(FName(TEXT("SphereTraceMultiForObjects")), false);

	// Perform sphere cast
	GetWorld()->SweepMultiByChannel(OutHits, StartPos, EndPos, FQuat::Identity, ECC_Visibility,
									FCollisionShape::MakeSphere(SphereCastRadius), Params);

	return true;
}


UIntenSelectable* UIntenSelectComponent::GetMaxScoreActor(const float DeltaTime)
{
	// Get cone origin and forward direction
	const FVector ConeOrigin = GetComponentTransform().GetLocation();
	const FVector ConeForward = GetComponentTransform().GetRotation().GetForwardVector();

	// Perform sphere cast to detect selectable actors
	TArray<FHitResult> OutHits;
	if (GetActorsFromSphereCast(ConeOrigin, OutHits))
	{
		// Iterate through hit results
		for (const FHitResult& Hit : OutHits)
		{
			const FVector PointToCheck = Hit.ImpactPoint;
			const float DistanceToActor = FVector::Dist(ConeOrigin, PointToCheck);

			const AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				// Check if the hit actor is selectable and within selection distance
				UIntenSelectable* Selectable = HitActor->FindComponentByClass<UIntenSelectable>();
				if (Selectable && Selectable->IsSelectable && DistanceToActor <= MaxSelectionDistance)
				{
					// Add to score map
					ScoreMap.FindOrAdd(Selectable, 0);
				}
			}
		}
	}

	// Variables for tracking the maximum score selectable
	UIntenSelectable* MaxScoreSelectable = nullptr;
	float MaxScore = TNumericLimits<float>::Min();
	TArray<UIntenSelectable*> RemoveList;
	TArray<TPair<UIntenSelectable*, FHitResult>> CandidateList;

	// Iterate through the score map
	for (TPair<UIntenSelectable*, float>& OldScoreEntry : ScoreMap)
	{
		if (!OldScoreEntry.Key)
		{
			continue;
		}

		// Calculate the new score and contact point
		TPair<FHitResult, float> NewScorePair = OldScoreEntry.Key->GetBestPointScorePair(
			ConeOrigin, ConeForward, ConeBackwardShiftDistance, SelectionConeAngle, OldScoreEntry.Value, DeltaTime);

		ContactPointMap.Add(OldScoreEntry.Key, NewScorePair.Key);
		const float DistanceToActor = FVector::Dist(ConeOrigin, NewScorePair.Key.ImpactPoint);

		// Check if the new score is valid and if the actor is still selectable
		const float Eps = 0.01;
		if (NewScorePair.Value <= 0.01 || DistanceToActor >= MaxSelectionDistance || !OldScoreEntry.Key->IsSelectable)
		{
			RemoveList.Add(OldScoreEntry.Key);
		}
		else
		{
			OldScoreEntry.Value = NewScorePair.Value;

			// Check if the new score exceeds the maximum score
			if (NewScorePair.Value > (1.0 - Eps) &&
				CheckPointInCone(ConeOrigin, ConeForward, NewScorePair.Key.ImpactPoint, SelectionConeAngle))
			{
				CandidateList.Emplace(OldScoreEntry.Key, NewScorePair.Key);
				MaxScore = NewScorePair.Value;
				MaxScoreSelectable = OldScoreEntry.Key;
			}
			else if (NewScorePair.Value > MaxScore &&
					 CheckPointInCone(ConeOrigin, ConeForward, NewScorePair.Key.ImpactPoint, SelectionConeAngle))
			{
				MaxScore = NewScorePair.Value;
				MaxScoreSelectable = OldScoreEntry.Key;
			}
		}
	}

	// Remove non-selectable actors from the maps
	for (UIntenSelectable* i : RemoveList)
	{
		ContactPointMap.Remove(i);
		ScoreMap.Remove(i);
	}

	// Select the nearest actor from the candidate list if available
	if (CandidateList.Num() > 0)
	{
		float DistanceToMaxScore =
			FVector::Distance(MaxScoreSelectable->GetOwner()->GetActorLocation(), GetComponentLocation());
		float Dist = TNumericLimits<float>::Max();
		for (const TPair<UIntenSelectable*, FHitResult>& Actor : CandidateList)
		{
			const float DistanceToCandidate = FVector::Distance(Actor.Value.ImpactPoint, GetComponentLocation());
			if (DistanceToCandidate < Dist)
			{
				MaxScoreSelectable = Actor.Key;
				Dist = DistanceToCandidate;
			}
		}
	}

	return MaxScoreSelectable;
}

//				RAYCASTING


void UIntenSelectComponent::HandleWidgetInteraction()
{
	// Get forward vector and origin of the component
	const FVector Forward = GetComponentTransform().GetRotation().GetForwardVector();
	const FVector Origin = GetComponentTransform().GetLocation();

	// Raytrace to find the first hit
	TOptional<FHitResult> Hit = RaytraceForFirstHit(Origin, Origin + Forward * MaxSelectionDistance);

	// If no hit, clear focus and return
	if (!Hit.IsSet())
	{
		IsWidgetInFocus = false;
		return;
	}

	// Set hit result and check if a widget is in focus
	SetCustomHitResult(Hit.GetValue());
	UWidgetComponent* FocusedWidget = Cast<UWidgetComponent>(Hit.GetValue().GetComponent());
	IsWidgetInFocus = (FocusedWidget != nullptr);

	// Handle widget events (commented out for now)
	/*
	if (IsWidgetInFocus)
	{
		if (FocusedWidget != LastFocusedWidget)
		{
			if (FocusedWidget->GetOwner()->Implements<UTargetable>())
			{
				ITargetable::Execute_OnTargetedEnter(FocusedWidget->GetOwner());
			}

			if (LastFocusedWidget != nullptr && LastFocusedWidget->GetOwner()->Implements<UTargetable>())
			{
				ITargetable::Execute_OnTargetedLeave(LastFocusedWidget->GetOwner());
			}
		}

		if (FocusedWidget->GetOwner()->Implements<UTargetable>())
		{
			ITargetable::Execute_OnTargeted(FocusedWidget->GetOwner(), Hit->Location);
		}
		LastFocusedWidget = FocusedWidget;

		if (FocusedWidget->GetOwner()->GetClass()->ImplementsInterface(UIntenSelectableWidget::StaticClass()))
		{
			FVector pos = IIntenSelectableWidget::Execute_GetCoordinates(FocusedWidget->GetOwner());
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "C++ Pos: " + pos.ToString());
			WidgetFocusPoint = pos;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "C++ Pos not available");
		}
	}
	*/
}


TOptional<FHitResult> UIntenSelectComponent::RaytraceForFirstHit(const FVector& Start, const FVector& End) const
{
	// Hit result to be filled by Line Trace function
	FHitResult Hit;

	// Set up collision query parameters
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()->GetUniqueID()); // Ignore the owner actor to prevent hitting itself

	// Perform line trace
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params))
	{
		return {Hit};
	}
	else
	{
		return {}; // No hit
	}
}


//				VISUALS

void UIntenSelectComponent::DrawSelectionCurve(const FVector& EndPoint) const
{
	// Get start point, forward vector, and set spline points
	const FVector StartPoint = GetComponentTransform().GetLocation();
	const FVector Forward = GetComponentTransform().GetRotation().GetForwardVector();

	SplineComponent->ClearSplinePoints(true);
	SplineMeshComponent->SetHiddenInGame(false);

	AddSplinePointsDefault(StartPoint, Forward, EndPoint);

	// Get spline start and end positions and tangents
	const FVector StartPosition = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
	const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
	const FVector EndPosition = SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local);
	const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(1, ESplineCoordinateSpace::Local);

	// Set start and end for spline mesh component
	SplineMeshComponent->SetStartAndEnd(StartPosition, StartTangent, EndPosition, EndTangent, true);
}


void UIntenSelectComponent::AddSplinePointsDefault(const FVector& StartPoint, const FVector& Forward,
												   const FVector& EndPoint) const
{
	// Add start and end points to the spline
	SplineComponent->AddSplineWorldPoint(StartPoint);
	SplineComponent->AddSplineWorldPoint(EndPoint);

	// Set spline point types
	SplineComponent->SetSplinePointType(0, ESplinePointType::Curve, true);
	SplineComponent->SetSplinePointType(1, ESplinePointType::Curve, true);

	// Calculate tangents for smooth curve
	const FVector StartToEnd = EndPoint - StartPoint;
	const FVector ForwardProjection = StartToEnd.ProjectOnTo(Forward);
	const FVector StartTangent = Forward * ForwardProjection.Size() * SplineCurvatureStrength;
	const FVector EndTangent = StartToEnd.GetSafeNormal();

	// Set tangents at spline points
	SplineComponent->SetTangentAtSplinePoint(0, StartTangent, ESplineCoordinateSpace::World, true);
	SplineComponent->SetTangentAtSplinePoint(1, EndTangent, ESplineCoordinateSpace::World, true);
}


void UIntenSelectComponent::UpdateForwardRay(const FVector& ReferencePoint) const
{
	// Check if transparency curve is available
	if (ForwardRayTransparencyCurve)
	{
		// Calculate cone forward vector and origin
		const FVector ConeForward = GetComponentTransform().GetRotation().GetForwardVector();
		const FVector ConeOrigin = GetComponentTransform().GetLocation() - (ConeForward * ConeBackwardShiftDistance);

		// Calculate angle to test point
		const FVector TestPointVector = (ReferencePoint - ConeOrigin).GetSafeNormal();
		const float AngleToTestPoint =
			FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ConeForward, TestPointVector)));

		// Calculate new transparency based on curve
		const float NewTransparency =
			ForwardRayTransparencyCurve->GetFloatValue(AngleToTestPoint / SelectionConeAngle) * DebugRayTransparency;

		// Set transparency parameter value in parameter collection
		if (ParameterCollectionInstance)
		{
			ParameterCollectionInstance->SetScalarParameterValue("Transparency", NewTransparency);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ParameterCollectionInstance is null!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ForwardRayTransparencyCurve is null!"));
	}
}


//				INPUT-HANDLING

void UIntenSelectComponent::OnFireDown()
{
	// Start interaction of WidgetInteractionComponent
	PressPointerKey(EKeys::LeftMouseButton);

	// Check if there is a current selection
	if (!CurrentSelection)
	{
		return;
	}

	// Handle action start events for current selection
	const FHitResult* GrabbedPoint = ContactPointMap.Find(CurrentSelection);
	if (GrabbedPoint)
	{
		CurrentSelection->HandleOnActionStartEvents(this);
		LastKnownSelection = CurrentSelection;
		LastKnownGrabPoint =
			LastKnownSelection->GetOwner()->GetRootComponent()->GetComponentTransform().InverseTransformPosition(
				GrabbedPoint->ImpactPoint);
	}
	else
	{
		LastKnownSelection = nullptr;
	}

	IsGrabbing = true;

	// Update transparency if required
	if (bDrawForwardRay && ParameterCollectionInstance)
	{
		ParameterCollectionInstance->SetScalarParameterValue("Transparency", 0);
	}
}


void UIntenSelectComponent::OnFireUp()
{
	// End interaction of WidgetInteractionComponent
	ReleasePointerKey(EKeys::LeftMouseButton);

	IsGrabbing = false;

	// Handle action end events for last known selection
	if (LastKnownSelection)
	{
		FInputActionValue v;
		LastKnownSelection->HandleOnActionEndEvents(this, v);
	}
}


//				SELECTION-HANDLING

void UIntenSelectComponent::SelectObject(UIntenSelectable* SelectableComponent, AActor* SelectedBy)
{
	// Set the current selection to the specified selectable component
	CurrentSelection = SelectableComponent;
}


void UIntenSelectComponent::Unselect()
{
	// Stop grabbing
	IsGrabbing = false;

	// Hide spline mesh component
	SplineMeshComponent->SetHiddenInGame(true);

	// Reset current selection
	CurrentSelection = nullptr;
}


void UIntenSelectComponent::SetActive(bool bNewActive, bool bReset)
{
	if (bNewActive)
	{
		// Show forward ray and spline mesh components
		ForwardRayMeshComponent->SetVisibility(true);
		SplineMeshComponent->SetVisibility(true);

		// Call superclass setActive function
		Super::SetActive(true, bReset);
	}
	else
	{
		// If there is a current selection, handle no actor selected
		if (CurrentSelection)
		{
			HandleNoActorSelected();
		}

		// If there is a last known selection, end the fire action
		if (LastKnownSelection)
		{
			OnFireUp();
		}

		// Hide forward ray and spline mesh components
		ForwardRayMeshComponent->SetVisibility(false);
		SplineMeshComponent->SetVisibility(false);

		// Call superclass setActive function
		Super::SetActive(false, bReset);
	}
}


//				TICK

void UIntenSelectComponent::HandleCooldown(const float DeltaTime)
{
	// Reduce feedback cooldown by delta time
	if (FeedbackCooldown > 0)
	{
		FeedbackCooldown -= DeltaTime;
	}
	// Ensure feedback cooldown does not go below 0
	else
	{
		FeedbackCooldown = 0;
	}
}

void UIntenSelectComponent::HandleActorSelected(UIntenSelectable* NewSelection)
{
	// Check if the new selection is different from the current selection
	if (NewSelection != CurrentSelection)
	{
		// If there is a current selection, handle hover end events
		if (CurrentSelection)
		{
			CurrentSelection->HandleOnHoverEndEvents(this);
		}

		// If there is a new selection, handle hover start events
		if (NewSelection)
		{
			const FHitResult* GrabbedPoint = ContactPointMap.Find(NewSelection);
			if (GrabbedPoint)
			{
				NewSelection->HandleOnHoverStartEvents(this, *GrabbedPoint);
			}
		}

		// Set the new selection as the current selection and trigger new selected event
		CurrentSelection = NewSelection;
		OnNewSelected(NewSelection);
	}

	// If there is a current selection, update forward ray and draw selection curve
	if (CurrentSelection)
	{
		const FHitResult* GrabbedPoint = ContactPointMap.Find(NewSelection);
		if (GrabbedPoint)
		{
			const FVector PointToDrawTo = ConvertNetVector(GrabbedPoint->ImpactPoint);

			if (bDrawForwardRay)
			{
				UpdateForwardRay(PointToDrawTo);
			}

			DrawSelectionCurve(PointToDrawTo);
		}
	}
}


FVector UIntenSelectComponent::ConvertNetVector(FVector_NetQuantize v)
{
	// Convert NetQuantize vector to FVector
	return FVector(v.X, v.Y, v.Z);
}


void UIntenSelectComponent::HandleNoActorSelected()
{
	// Hide spline mesh component
	SplineMeshComponent->SetHiddenInGame(true);

	// If there is a current selection, handle hover end events and reset current selection
	if (CurrentSelection)
	{
		CurrentSelection->HandleOnHoverEndEvents(this);
		CurrentSelection = nullptr;
	}

	// Reset transparency parameter if drawing forward ray
	if (bDrawForwardRay && ParameterCollectionInstance)
	{
		ParameterCollectionInstance->SetScalarParameterValue("Transparency", DebugRayTransparency);
	}
}


void UIntenSelectComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
										  FActorComponentTickFunction* ThisTickFunction)
{
	// Call parent tick function
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle cooldown for feedback
	HandleCooldown(DeltaTime);

	// Get the new selection based on current parameters
	UIntenSelectable* const NewSelection = GetMaxScoreActor(DeltaTime);

	// If currently grabbing an object, update selection curve and check for angle constraints
	if (IsGrabbing && LastKnownSelection)
	{
		const FVector GrabPointWorld =
			LastKnownSelection->GetOwner()->GetRootComponent()->GetComponentTransform().TransformPosition(
				LastKnownGrabPoint);
		DrawSelectionCurve(GrabPointWorld);

		const FVector ConeOrigin = GetComponentLocation();
		const FVector ConeForward = GetForwardVector().GetSafeNormal();

		if (!CheckPointInCone(ConeOrigin, ConeForward, GrabPointWorld, MaxClickStickAngle))
		{
			OnFireUp();
		}

		return;
	}
	else if (CurrentSelection && ContactPointMap.Contains(CurrentSelection))
	{
		const FVector GrabbedPoint = ConvertNetVector(ContactPointMap.Find(CurrentSelection)->ImpactPoint);
		DrawSelectionCurve(GrabbedPoint);
	}

	// Handle widget interaction
	IsWidgetInFocus = false;
	if (IsWidgetInFocus)
	{
		HandleNoActorSelected();

		const FVector PointToDrawTo = WidgetFocusPoint;

		if (bDrawForwardRay)
		{
			UpdateForwardRay(PointToDrawTo);
		}

		DrawSelectionCurve(PointToDrawTo);
	}
	else
	{
		// If there is a new selection, handle it; otherwise, handle no actor selected
		if (NewSelection)
		{
			HandleActorSelected(NewSelection);
		}
		else
		{
			HandleNoActorSelected();
		}
	}
}
