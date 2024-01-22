#include "Pawn/IntenSelectComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"
#include "Materials/MaterialExpressionStrata.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/MessageDialog.h"
#include "UI/IntenSelectableWidget.h"
#include "Templates/Tuple.h"


//				INITIALIZATION

// Sets default values for this component's properties
UIntenSelectComponent::UIntenSelectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bShowDebug = false; //otherwise the WidgetInteractionComponent debug vis is shown
	InteractionSource = EWidgetInteractionSource::Custom; //can also be kept at default (World), this way, however, we efficiently reuse the line traces

	ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultConeMesh(TEXT("StaticMesh'/RWTHVRToolkit/IntenSelect/DebugConeMesh.DebugConeMesh'"));
	this->DebugConeMesh = DefaultConeMesh.Object;
	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultConeMeshMaterial(TEXT("Material'/RWTHVRToolkit/IntenSelect/DebugConeMaterial.DebugConeMaterial'"));
	this->DebugConeMaterial = DefaultConeMeshMaterial.Object;

	ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultSplineMesh(TEXT("StaticMesh'/RWTHVRToolkit/IntenSelect/sectionedCubeMesh.sectionedCubeMesh'"));
	this->SplineMesh = DefaultSplineMesh.Object;
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultForwardRayMesh(TEXT("StaticMesh'/RWTHVRToolkit/IntenSelect/RayMesh.RayMesh'"));
	this->ForwardRayMesh = DefaultForwardRayMesh.Object;

	ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultSplineMaterial(TEXT("Material'/RWTHVRToolkit/IntenSelect/SelectionSplineMaterial.SelectionSplineMaterial'"));
	this->SplineMaterial = DefaultSplineMaterial.Object;
	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultForwardRayMaterial(TEXT("Material'/RWTHVRToolkit/IntenSelect/ForwadRayMaterial.ForwadRayMaterial'"));
	this->ForwardRayMaterial = DefaultForwardRayMaterial.Object;

	ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Curve> DefaultSelectionFeedbackHaptic(TEXT("HapticFeedbackEffect_Curve'/RWTHVRToolkit/IntenSelect/OnSelectHapticFeedback.OnSelectHapticFeedback'"));
	this->SelectionFeedbackHaptic = DefaultSelectionFeedbackHaptic.Object;
	
	ConstructorHelpers::FObjectFinder<USoundBase> DefaultOnSelectSound(TEXT("SoundWave'/RWTHVRToolkit/IntenSelect/OnSelectSound.OnSelectSound'"));
	this->OnSelectSound = DefaultOnSelectSound.Object;
	
	ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> DefaultMaterialParamCollection(TEXT("MaterialParameterCollection'/RWTHVRToolkit/IntenSelect/ForwardRayParams.ForwardRayParams'"));
	this->MaterialParamCollection = DefaultMaterialParamCollection.Object;
	
	ConstructorHelpers::FObjectFinder<UCurveFloat> DefaultForwardRayTransparencyCurve(TEXT("CurveFloat'/RWTHVRToolkit/IntenSelect/ForwardRayTransparencyCurve.ForwardRayTransparencyCurve'"));
	this->ForwardRayTransparencyCurve = DefaultForwardRayTransparencyCurve.Object;
	ConstructorHelpers::FObjectFinder<UInputAction> InputActionClick(TEXT("IntenSelectClick'/RWTHVRToolkit/IntenSelect/IntenSelectClick.IntenSelectClick'"));
	this->InputClick = InputActionClick.Object;
}

// Called when the game starts
void UIntenSelectComponent::BeginPlay()
{
	Super::BeginPlay();

	this->InitSplineComponent();
	this->InitSplineMeshComponent();
	this->InitForwardRayMeshComponent();
	this->InitDebugConeMeshComponent();
	this->InitInputBindings();
	this->InitMaterialParamCollection();	
	
	this->SphereCastRadius = CalculateSphereCastRadius();
	this->InteractionDistance = this->MaxSelectionDistance;

	this->SetActive(SetActiveOnStart, false);
}

void UIntenSelectComponent::InitInputBindings(){
	const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	UInputComponent* PlayerInputComponent = PC->InputComponent;
	UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	// Bind the actions
	PEI->BindAction(InputClick, ETriggerEvent::Started, this, &UIntenSelectComponent::OnFireDown);
	PEI->BindAction(InputClick, ETriggerEvent::Completed, this, &UIntenSelectComponent::OnFireUp);

	// Clear out existing mapping, and add our mapping
	//Subsystem->ClearAllMappings();
	//Subsystem->AddMappingContext(InputMapping, 0);

	
	// const auto InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	// if(!InputComponent)
	// {
	// 	const FString Message = "There is no InputComponent attached to the same Actor as IntenSelect!";
	// #if WITH_EDITOR
	// 	const FText Title = FText::FromString(FString("ERROR"));
	// 	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), &Title);
	// #endif
	// 	
	// 	UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
	// 	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	// 	return;
	// }
	
	// InputComponent->BindAction("Fire", IE_Pressed, this, &UIntenSelectComponent::OnFireDown);
	// InputComponent->BindAction("Fire", IE_Released, this, &UIntenSelectComponent::OnFireUp);
}

void UIntenSelectComponent::InitSplineComponent()
{
	SplineComponent = NewObject<USplineComponent>(this, TEXT("SplineComponent"));

	if(SplineComponent)
	{
		SplineComponent->SetupAttachment(this);
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->RegisterComponent();
		SplineComponent->CreationMethod = EComponentCreationMethod::Instance;

	}else
	{
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
	SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), TEXT("SplineMeshComponent"));
	if(SplineMeshComponent)
	{
		SplineMeshComponent->SetupAttachment(this);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->RegisterComponent();
		SplineMeshComponent->CreationMethod = EComponentCreationMethod::Instance;

		if (SplineMesh)
		{
			SplineMeshComponent->SetStaticMesh(SplineMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineMesh not set!"));
		}

		if (SplineMaterial)
		{
			SplineMeshComponent->SetMaterial(0, SplineMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineMesh material not set! Using default material instead."));
		}

		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::Z);
		SplineMeshComponent->CastShadow = false;
		
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Error while spawning SplineMeshComponent!"))
	}
}

void UIntenSelectComponent::InitForwardRayMeshComponent()
{
	ForwardRayMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("ForwardRay"));

	if(ForwardRayMeshComponent)
	{
		ForwardRayMeshComponent->SetupAttachment(this);
		ForwardRayMeshComponent->SetMobility((EComponentMobility::Movable));
		ForwardRayMeshComponent->RegisterComponent();
		ForwardRayMeshComponent->CreationMethod = EComponentCreationMethod::Instance;

		ForwardRayMeshComponent->SetCastShadow(false);
		ForwardRayMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		const float MeshLength = MaxSelectionDistance > 1000 ? 1000 : MaxSelectionDistance;
		ForwardRayMeshComponent->SetRelativeScale3D(FVector(MeshLength, ForwardRayWidth, ForwardRayWidth));
		ForwardRayMeshComponent->SetRelativeLocation(FVector(MeshLength * 50, 0, 0));

		//const ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
		if(ForwardRayMesh)
		{
			ForwardRayMeshComponent->SetStaticMesh(ForwardRayMesh);
		}else
		{
			UE_LOG(LogTemp, Warning, TEXT("Mesh for RayComponent not set!"));
		}

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ForwardRayMaterial, ForwardRayMeshComponent);
		this->ForwardRayMeshComponent->SetMaterial(0, DynamicMaterial);

		ForwardRayMeshComponent->SetHiddenInGame(!bDrawForwardRay);

	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Error while spawning ForwardRayMesh component!"));
	}
}

void UIntenSelectComponent::InitMaterialParamCollection()
{
	if(MaterialParamCollection)
	{
		this->ParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialParamCollection);
		if(this->ParameterCollectionInstance)
		{
			this->ParameterCollectionInstance->SetScalarParameterValue("Transparency", DebugRayTransparency);
		}else
		{
			UE_LOG(LogTemp, Warning, TEXT("MaterialParameterCollection required for rendering of IntenSelect could not be found!"))
		}
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("MaterialParameterCollection required for InteSelect visualization is not set!"));
	}
}

void UIntenSelectComponent::InitDebugConeMeshComponent()
{
	DebugConeMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("DebugCone"));

	if(DebugConeMeshComponent)
	{
		DebugConeMeshComponent->SetupAttachment(this);
		DebugConeMeshComponent->SetMobility(EComponentMobility::Movable);
		DebugConeMeshComponent->RegisterComponent();
		DebugConeMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
		

		FTransform ConeTransform = DebugConeMeshComponent->GetRelativeTransform();
		const float ConeScale = MaxSelectionDistance / 50 * FMath::Tan(FMath::DegreesToRadians(SelectionConeAngle));
		ConeTransform.SetScale3D(FVector(ConeScale, ConeScale, MaxSelectionDistance / 100));

		DebugConeMeshComponent->SetRelativeTransform(ConeTransform);
		DebugConeMeshComponent->SetRelativeLocation(FVector(MaxSelectionDistance - ConeBackwardShiftDistance, 0, 0), false);
		DebugConeMeshComponent->SetRelativeRotation(DebugConeRotation, false);
		DebugConeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (DebugConeMesh)
		{
			DebugConeMeshComponent->SetStaticMesh(DebugConeMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DebugCone mesh not set!"))
		}
		if (DebugConeMaterial)
		{
			DebugConeMeshComponent->SetMaterial(0, DebugConeMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DebugCone material not set! Using default material instead."))
		}

		DebugConeMeshComponent->SetVisibility(bDrawDebugCone);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Error while spawning DebugCone component!"))
	}
}


//				SCORING FUNCTIONS

float UIntenSelectComponent::CalculateSphereCastRadius() const
{
	return FMath::Tan(FMath::DegreesToRadians(SelectionConeAngle)) * MaxSelectionDistance;
}

bool UIntenSelectComponent::CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward, const FVector PointToTest, const float Angle) const
{
	const FVector ShiftedStartOriginPoint = ConeStartPoint - (ConeForward * ConeBackwardShiftDistance);
	const FVector DirectionToTestPoint = (PointToTest - ShiftedStartOriginPoint).GetSafeNormal();

	const float AngleToTestPoint = FMath::RadiansToDegrees(FMath::Acos((FVector::DotProduct(ConeForward ,DirectionToTestPoint))));
	
	return AngleToTestPoint <= Angle;
}

void UIntenSelectComponent::OnNewSelected_Implementation(UIntenSelectable* Selection)
{
	CurrentSelection = Selection;
		
	if(FeedbackCooldown == 0)
	{
		//UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayHapticEffect(SelectionFeedbackHaptic, EControllerHand::Right, 0.1, false);
		UGameplayStatics::PlaySound2D(GetWorld(), OnSelectSound);
		FeedbackCooldown = 0.1;
	}
}

bool UIntenSelectComponent::GetActorsFromSphereCast(const FVector& SphereCastStart, TArray<FHitResult>& OutHits) const
{
	const FVector StartPos = SphereCastStart + (GetComponentTransform().GetRotation().GetForwardVector() * SphereCastRadius);
	const FVector EndPos = StartPos + (this->GetComponentTransform().GetRotation().GetForwardVector() * (MaxSelectionDistance));

	const FCollisionQueryParams Params = FCollisionQueryParams(FName(TEXT("SphereTraceMultiForObjects")), false);
	//GetWorld()->SweepMultiByChannel(OutHits, StartPos, EndPos, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereCastRadius), Params);
	
	GetWorld()->SweepMultiByChannel(OutHits, StartPos, EndPos, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereCastRadius), Params);
	// UKismetSystemLibrary::SphereTraceMulti(GetWorld(),StartPos,EndPos,SphereCastRadius,ETraceTypeQuery::TraceTypeQuery1,false,{},EDrawDebugTrace::ForOneFrame,OutHits,true);
	return true;
}

UIntenSelectable* UIntenSelectComponent::GetMaxScoreActor(const float DeltaTime)
{
	const FVector ConeOrigin = this->GetComponentTransform().GetLocation();
	const FVector ConeForward = this->GetComponentTransform().GetRotation().GetForwardVector();

	TArray<FHitResult> OutHits;
	if (GetActorsFromSphereCast(ConeOrigin, OutHits)) {
		for (const FHitResult& Hit : OutHits)
		{
			const FVector PointToCheck = Hit.ImpactPoint;
			const float DistanceToActor = FVector::Dist(ConeOrigin, PointToCheck);

			const AActor* HitActor = Hit.GetActor();
			if(HitActor)
			{
				const auto Selectable = HitActor->FindComponentByClass<UIntenSelectable>();

				if(Selectable && Selectable->IsSelectable
					&& DistanceToActor <= MaxSelectionDistance)
				{
					ScoreMap.FindOrAdd(Selectable, 0);
				}
			}
		}
	}

	UIntenSelectable* MaxScoreSelectable = nullptr;
	float MaxScore = TNumericLimits<float>::Min();
	TArray<UIntenSelectable*> RemoveList;
	TArray<TPair<UIntenSelectable*, FHitResult>> CandidateList;
	
	for (TTuple<UIntenSelectable*, float>& OldScoreEntry : ScoreMap)
	{
		//GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black,  OldScoreEntry.Key->GetOwner()->GetName() + " - Score: " + FString::SanitizeFloat(OldScoreEntry.Value));
		if(!OldScoreEntry.Key)
		{
			continue;
		}

		TPair<FHitResult, float> NewScorePair = OldScoreEntry.Key->GetBestPointScorePair(
			ConeOrigin, ConeForward, ConeBackwardShiftDistance, SelectionConeAngle, OldScoreEntry.Value, DeltaTime);
		
		ContactPointMap.Add(OldScoreEntry.Key, NewScorePair.Key);
		const float DistanceToActor = FVector::Dist(ConeOrigin, NewScorePair.Key.ImpactPoint);

		const float Eps = 0.01;
		if (NewScorePair.Value <= 0.01 || DistanceToActor >= MaxSelectionDistance || !OldScoreEntry.Key->IsSelectable)
		{
			RemoveList.Add(OldScoreEntry.Key);
		}
		else
		{
			OldScoreEntry.Value = NewScorePair.Value;

			if (NewScorePair.Value > (1.0 - Eps) && this->CheckPointInCone(ConeOrigin, ConeForward, NewScorePair.Key.ImpactPoint, SelectionConeAngle))
			{
				CandidateList.Emplace(OldScoreEntry.Key, NewScorePair.Key);
				MaxScore = NewScorePair.Value;
				MaxScoreSelectable = OldScoreEntry.Key;
			}
			else if (NewScorePair.Value > MaxScore && this->CheckPointInCone(ConeOrigin, ConeForward, NewScorePair.Key.ImpactPoint, SelectionConeAngle))
			{
				MaxScore = NewScorePair.Value;
				MaxScoreSelectable = OldScoreEntry.Key;
			}
		}
	}

	for (const UIntenSelectable* i : RemoveList)
	{
		ContactPointMap.Remove(i);
		ScoreMap.Remove(i);
	}
	if (CandidateList.Num() > 0)
	{
		auto DistanceToMaxScore = FVector::Distance(MaxScoreSelectable->GetOwner()->GetActorLocation(),GetComponentLocation());
		auto Dist = TNumericLimits<float>::Max();
		for(const auto Actor : CandidateList)
		{
			const auto DistanceToCandidate = FVector::Distance(Actor.Value.ImpactPoint, GetComponentLocation());
			if(DistanceToCandidate < Dist)
			{
				MaxScoreSelectable = Actor.Key;
				Dist = DistanceToCandidate;
			}
		}
	}

	return  MaxScoreSelectable;
}
//				RAYCASTING


void UIntenSelectComponent::HandleWidgetInteraction()
{
	const FVector Forward = this->GetComponentTransform().GetRotation().GetForwardVector();
	const FVector Origin = this->GetComponentTransform().GetLocation();
	
	TOptional<FHitResult> Hit = RaytraceForFirstHit(Origin, Origin + Forward * MaxSelectionDistance);
	
	if (!Hit.IsSet())
	{
		IsWidgetInFocus = false;
		return;
	}
	
	SetCustomHitResult(Hit.GetValue());
	UWidgetComponent* FocusedWidget = Cast<UWidgetComponent>(Hit.GetValue().GetComponent());
	IsWidgetInFocus = (FocusedWidget != nullptr);


	/*
	if(IsWidgetInFocus)
	{
		if (FocusedWidget != LastFocusedWidget)
		{
			//We can always execute the enter event as we are sure that a hit occured
			if (FocusedWidget->GetOwner()->Implements<UTargetable>())
			{
				ITargetable::Execute_OnTargetedEnter(FocusedWidget->GetOwner());
			}

			//Only execute the Leave Event if there was an actor that was focused previously
			if (LastFocusedWidget != nullptr && LastFocusedWidget->GetOwner()->Implements<UTargetable>())
			{
				ITargetable::Execute_OnTargetedLeave(LastFocusedWidget->GetOwner());
			}
		}

		// for now uses the same distance as clicking
		if (FocusedWidget->GetOwner()->Implements<UTargetable>())
		{
			ITargetable::Execute_OnTargeted(FocusedWidget->GetOwner(), Hit->Location);
		}
		LastFocusedWidget = FocusedWidget;

		if(FocusedWidget->GetOwner()->GetClass()->ImplementsInterface(UIntenSelectableWidget::StaticClass()))
		{
			FVector pos = IIntenSelectableWidget::Execute_GetCoordinates(FocusedWidget->GetOwner());
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "C++ Pos: " + pos.ToString());
			WidgetFocusPoint = pos;
		}else
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "C++ Pos not available");
		}
	}*/
}

TOptional<FHitResult> UIntenSelectComponent::RaytraceForFirstHit(const FVector& Start, const FVector& End) const
{
	// will be filled by the Line Trace Function
	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()->GetUniqueID()); // prevents actor hitting itself
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params))
	{
		return {Hit};

	}else
	{
		return {};
	}
}


//				VISUALS

void UIntenSelectComponent::DrawSelectionCurve(const FVector& EndPoint) const
{
	const FVector StartPoint = this->GetComponentTransform().GetLocation();
	const FVector Forward = this->GetComponentTransform().GetRotation().GetForwardVector();

	SplineComponent->ClearSplinePoints(true);
	SplineMeshComponent->SetHiddenInGame(false);
	
	AddSplinePointsDefault(StartPoint, Forward, EndPoint);

	const FVector StartPosition = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
	const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local);
	const FVector EndPosition = SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local);
	const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(1, ESplineCoordinateSpace::Local);

	SplineMeshComponent->SetStartAndEnd(StartPosition, StartTangent, EndPosition, EndTangent, true);
}

void UIntenSelectComponent::AddSplinePointsDefault(const FVector& StartPoint, const FVector& Forward, const FVector& EndPoint) const
{
	SplineComponent->AddSplineWorldPoint(StartPoint);

	const FVector StartToEnd = EndPoint - StartPoint;
	const FVector ForwardProjection = StartToEnd.ProjectOnTo(Forward);

	SplineComponent->AddSplineWorldPoint(EndPoint);

	SplineComponent->SetSplinePointType(0, ESplinePointType::Curve, true);
	SplineComponent->SetSplinePointType(1, ESplinePointType::Curve, true);
	
	SplineComponent->SetTangentAtSplinePoint(0, Forward * ForwardProjection.Size() * SplineCurvatureStrength, ESplineCoordinateSpace::World, true);
	SplineComponent->SetTangentAtSplinePoint(1, StartToEnd.GetSafeNormal(), ESplineCoordinateSpace::World, true);
}

void UIntenSelectComponent::UpdateForwardRay(const FVector& ReferencePoint) const
{
	if(ForwardRayTransparencyCurve)
	{
		const FVector ConeForward = this->GetComponentTransform().GetRotation().GetForwardVector();
		const FVector ConeOrigin = this->GetComponentTransform().GetLocation() - (ConeForward * ConeBackwardShiftDistance);

		const FVector TestPointVector = (ReferencePoint - ConeOrigin).GetSafeNormal();
		const float AngleToTestPoint = FMath::RadiansToDegrees(FMath::Acos((FVector::DotProduct(ConeForward, TestPointVector))));
		
		const float NewTransparency = ForwardRayTransparencyCurve->GetFloatValue(AngleToTestPoint / SelectionConeAngle) * DebugRayTransparency;
		ParameterCollectionInstance->SetScalarParameterValue("Transparency", NewTransparency);
	}
}

//				INPUT-HANDLING

void UIntenSelectComponent::OnFireDown()
{
	//start interaction of WidgetInteractionComponent
	PressPointerKey(EKeys::LeftMouseButton);

	if(!CurrentSelection)
	{
		return;
	}
	
	if(CurrentSelection)
	{
		const FHitResult GrabbedPoint = *ContactPointMap.Find(CurrentSelection);
		CurrentSelection->HandleOnClickStartEvents(this);
		LastKnownSelection = CurrentSelection;
		LastKnownGrabPoint = LastKnownSelection->GetOwner()->GetRootComponent()->GetComponentTransform().InverseTransformPosition(GrabbedPoint.ImpactPoint);
	}else
	{
		LastKnownSelection = nullptr;
	}
	
	IsGrabbing = true;
		
	if (bDrawForwardRay && ParameterCollectionInstance)
	{
		ParameterCollectionInstance->SetScalarParameterValue("Transparency", 0);
	}
}

void UIntenSelectComponent::OnFireUp()
{
	//end interaction of WidgetInteractionComponent
	ReleasePointerKey(EKeys::LeftMouseButton);

	IsGrabbing = false;
	
	if (LastKnownSelection)
	{
		FInputActionValue v;
		LastKnownSelection->HandleOnClickEndEvents(this, v);
	}
}

//				SELECTION-HANDLING

void UIntenSelectComponent::SelectObject(UIntenSelectable* SelectableComponent, AActor* SelectedBy) {
	CurrentSelection = SelectableComponent;
}

void UIntenSelectComponent::Unselect()
{
	IsGrabbing = false;
	
	SplineMeshComponent->SetHiddenInGame(true);

	CurrentSelection = nullptr;
	this->CurrentSelection = nullptr;
}

void UIntenSelectComponent::SetActive(bool bNewActive, bool bReset)
{
	if(bNewActive)
	{
		ForwardRayMeshComponent->SetVisibility(true);
		SplineMeshComponent->SetVisibility(true);
		
		Super::SetActive(true, bReset);
	}else
	{
		if(CurrentSelection)
		{
			HandleNoActorSelected();
		}

		if(LastKnownSelection)
		{
			OnFireUp();
		}

		ForwardRayMeshComponent->SetVisibility(false);
		SplineMeshComponent->SetVisibility(false);
		
		Super::SetActive(false, bReset);
	}
}

//				TICK

void UIntenSelectComponent::HandleCooldown(const float DeltaTime)
{
	if (FeedbackCooldown > 0)
	{
		FeedbackCooldown -= DeltaTime;
	}
	else
	{
		FeedbackCooldown = 0;
	}
}

void UIntenSelectComponent::HandleGrabbing(const float DeltaTime) const
{
	
}

void UIntenSelectComponent::HandleActorSelected(UIntenSelectable* NewSelection)
{
	if (NewSelection != CurrentSelection)
	{
		if(CurrentSelection)
		{
			CurrentSelection->HandleOnSelectEndEvents(this);
		}
		
		if(NewSelection)
		{
			UIntenSelectable* NewIntenSelectable = NewSelection;
			const FHitResult GrabbedPoint = *ContactPointMap.Find(NewIntenSelectable);
			NewIntenSelectable->HandleOnSelectStartEvents(this, GrabbedPoint);
		}

		CurrentSelection = NewSelection;
		OnNewSelected(NewSelection);
	}
	
	if(CurrentSelection)
	{
		const UIntenSelectable* NewIntenSelectable = NewSelection;
		const auto V_Net = ContactPointMap.Find(NewIntenSelectable)->ImpactPoint;
		const FVector PointToDrawTo = ConvertNetVector(V_Net);

		if (bDrawForwardRay)
		{
			UpdateForwardRay(PointToDrawTo);
		}

		DrawSelectionCurve(PointToDrawTo);
	}
}

FVector UIntenSelectComponent::ConvertNetVector(FVector_NetQuantize v)
{
	FVector Result;
	Result.X = v.X;
	Result.Y = v.Y;
	Result.Z = v.Z;
	return Result;
}


void UIntenSelectComponent::HandleNoActorSelected()
{
	SplineMeshComponent->SetHiddenInGame(true);
	
	if(CurrentSelection)
	{
		CurrentSelection->HandleOnSelectEndEvents(this);
	}
	
	if (bDrawForwardRay && ParameterCollectionInstance)
	{
		ParameterCollectionInstance->SetScalarParameterValue("Transparency", DebugRayTransparency);
	}
	CurrentSelection = nullptr;
}

void UIntenSelectComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	this->HandleCooldown(DeltaTime);
	UIntenSelectable* const NewSelection = GetMaxScoreActor(DeltaTime);
	
	if(IsGrabbing && LastKnownSelection)
	{
		const FVector GrabPointWorld = LastKnownSelection->GetOwner()->GetRootComponent()->GetComponentTransform().
		                                                   TransformPosition(LastKnownGrabPoint);
		DrawSelectionCurve(GrabPointWorld);

		const FVector ConeOrigin = this->GetComponentLocation();
		const FVector ConeForward = this->GetForwardVector().GetSafeNormal();
		
		if(!this->CheckPointInCone(ConeOrigin, ConeForward, GrabPointWorld, MaxClickStickAngle))
		{
			OnFireUp();
		}
		
		return;
	}else if(CurrentSelection && ContactPointMap.Contains(CurrentSelection))
	{
		const FVector GrabbedPoint = ConvertNetVector(ContactPointMap.Find(CurrentSelection)->ImpactPoint);
		DrawSelectionCurve(GrabbedPoint);
	}
	
	//this->HandleWidgetInteraction();
	IsWidgetInFocus = false;
	if(IsWidgetInFocus)
	{
		//GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, "Widget focused");
		HandleNoActorSelected();

		const FVector PointToDrawTo = WidgetFocusPoint;

		if (bDrawForwardRay)
		{
			UpdateForwardRay(PointToDrawTo);
		}

		DrawSelectionCurve(PointToDrawTo);
	}else
	{
		if (NewSelection)
		{
			//GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, "Focused Actor:" + NewSelection->GetName());
			HandleActorSelected(NewSelection);
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, "No Actor in Focus");
			HandleNoActorSelected();
		}
	}
}
