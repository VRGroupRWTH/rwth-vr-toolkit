#include "Pawn/RaycastSelectComponent.h"

#include <string>

#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "EditorFramework/AssetImportData.h"
#include "GameFramework/Character.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"
#include "Interaction/RaycastSelectable.h"
#include "Misc/MessageDialog.h"


//				INITIALIZATION

// Sets default values for this component's properties
URaycastSelectComponent::URaycastSelectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bShowDebug = false; //otherwise the WidgetInteractionComponent debug vis is shown
	InteractionSource = EWidgetInteractionSource::Custom; //can also be kept at default (World), this way, however, we efficiently reuse the line traces

	ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultForwardRayMesh(TEXT("StaticMesh'/RWTHVRToolkit/IntenSelect/RayMesh.RayMesh'"));
	this->ForwardRayMesh = DefaultForwardRayMesh.Object;

	ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultForwardRayMaterial(TEXT("Material'/RWTHVRToolkit/IntenSelect/ForwadRayMaterial.ForwadRayMaterial'"));
	this->ForwardRayMaterial = DefaultForwardRayMaterial.Object;

	ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Curve> DefaultSelectionFeedbackHaptic(TEXT("HapticFeedbackEffect_Curve'/RWTHVRToolkit/IntenSelect/OnSelectHapticFeedback.OnSelectHapticFeedback'"));
	this->SelectionFeedbackHaptic = DefaultSelectionFeedbackHaptic.Object;
	
	ConstructorHelpers::FObjectFinder<USoundBase> DefaultOnSelectSound(TEXT("SoundWave'/RWTHVRToolkit/IntenSelect/OnSelectSound.OnSelectSound'"));
	this->OnSelectSound = DefaultOnSelectSound.Object;
}

// Called when the game starts
void URaycastSelectComponent::BeginPlay()
{
	Super::BeginPlay();

	this->InitForwardRayMeshComponent();
	this->InitInputBindings();
	this->InteractionDistance = this->MaxSelectionDistance;

	this->SetActive(SetActiveOnStart, false);
}

void URaycastSelectComponent::InitInputBindings(){
	const auto InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if(!InputComponent)
	{
		const FString Message = "There is no InputComponent attached to the same Actor as the RaycastComponent!";
	#if WITH_EDITOR
		const FText Title = FText::FromString(FString("ERROR"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), &Title);
	#endif
		
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
		return;
	}
	
	// InputComponent->BindAction("Fire", IE_Pressed, this, &URaycastSelectComponent::OnFireDown);
	// InputComponent->BindAction("Fire", IE_Released, this, &URaycastSelectComponent::OnFireUp);
}

void URaycastSelectComponent::InitForwardRayMeshComponent()
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
		ForwardRayMeshComponent->SetRelativeScale3D(FVector(MeshLength, 0.01, 0.01));
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

//				SCORING FUNCTIONS

bool URaycastSelectComponent::CheckPointInCone(const FVector ConeStartPoint, const FVector ConeForward, const FVector PointToTest, const float ConeAngle)
{
	const FVector DirectionToTestPoint = (PointToTest - ConeStartPoint).GetSafeNormal();
	const float AngleToTestPoint = FMath::RadiansToDegrees(FMath::Acos((FVector::DotProduct(ConeForward ,DirectionToTestPoint))));
	
	return AngleToTestPoint <= ConeAngle;
}

void URaycastSelectComponent::OnNewSelected_Implementation(UIntenSelectable* Selection)
{
	UGameplayStatics::PlaySound2D(GetWorld(), OnSelectSound);
}

//				RAYCASTING


TOptional<FHitResult> URaycastSelectComponent::RaytraceForFirstHit(const FVector& Start, const FVector& End) const
{
	// will be filled by the Line Trace Function
	TArray<FHitResult> Hits;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()->GetUniqueID()); // prevents actor hitting itself
	GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECollisionChannel::ECC_Visibility, Params);
	if (Hits.Num() > 0)
	{
		return Hits[0];
	}
	return {};
}


//				INPUT-HANDLING

void URaycastSelectComponent::OnFireDown()
{
	//start interaction of WidgetInteractionComponent
	PressPointerKey(EKeys::LeftMouseButton);

	IsGrabbing = true;

	if(CurrentSelection)
	{
		CurrentSelection->HandleOnClickStartEvents(this, CurrentSelectionPoint);
		LastKnownGrab = CurrentSelection;
		LastKnownGrabPoint = CurrentSelectionPoint;
	}
}

void URaycastSelectComponent::OnFireUp()
{
	//end interaction of WidgetInteractionComponent
	ReleasePointerKey(EKeys::LeftMouseButton);

	if(IsGrabbing)
	{
		IsGrabbing = false;

		if(LastKnownGrab)
		{
			LastKnownGrab->HandleOnClickEndEvents(this);
		}
	}
}

//				SELECTION-HANDLING

void URaycastSelectComponent::SetActive(bool bNewActive, bool bReset)
{
	if(bNewActive)
	{
		ForwardRayMeshComponent->SetVisibility(true);
		Super::SetActive(true, bReset);
	}else
	{
		if(CurrentSelection)
		{
			CurrentSelection->HandleOnSelectEndEvents(this);
			CurrentSelection = nullptr;
		}

		if(LastKnownGrab)
		{
			OnFireUp();
		}

		ForwardRayMeshComponent->SetVisibility(false);
		
		Super::SetActive(false, bReset);
	}
}

//				TICK

void URaycastSelectComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	const FVector RaycastOrigin = this->GetComponentLocation();
	const FVector RaycastForward = this->GetForwardVector().GetSafeNormal();
	const FVector RaycastEnd = RaycastOrigin + (RaycastForward * MaxSelectionDistance);

	if(bDrawDebugRay)
	{
		DrawDebugLine(GetWorld(), RaycastOrigin, RaycastEnd, FColor::Green, false, 0, 0, 5);
	}
	TOptional<FHitResult> HitResult = this->RaytraceForFirstHit(RaycastOrigin, RaycastEnd);

	if(IsGrabbing)
	{
		if(!this->CheckPointInCone(RaycastOrigin, RaycastForward, LastKnownGrabPoint, MaxClickStickAngle))
		{
			OnFireUp();
		}
		return;
	}
	
	URaycastSelectable* NewSelection = nullptr;
	if(HitResult.IsSet())
	{
		AActor* HitActor = HitResult.GetValue().Actor.Get();
		if(HitActor)
		{
			NewSelection = HitActor->FindComponentByClass<URaycastSelectable>();
			if(bShowHitsOnScreen)
			{
				if(!NewSelection)
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "Actor hit that has no RaycastSelectable!" + HitActor->GetName());
				}else
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, HitActor->GetName());
				}
			}
		}
	}else if(bShowHitsOnScreen)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Black, "nothing hit");
	}
	
	if(NewSelection)
	{
		//New valid selection

		if(CurrentSelection)
		{
			if(CurrentSelection != NewSelection)
			{
				CurrentSelection->HandleOnSelectEndEvents(this);
				CurrentSelectionPoint = HitResult->ImpactPoint;
				
				NewSelection->HandleOnSelectStartEvents(this, CurrentSelectionPoint);
				CurrentSelection = NewSelection;
			}
		}else
		{
			NewSelection->HandleOnSelectStartEvents(this, HitResult->ImpactPoint);
			CurrentSelection = NewSelection;
			CurrentSelectionPoint = HitResult->ImpactPoint;
		}
	}else
	{
		//No new valid selection

		if(CurrentSelection)
		{
			CurrentSelection->HandleOnSelectEndEvents(this);
			CurrentSelection = nullptr;
		}
	}
}
