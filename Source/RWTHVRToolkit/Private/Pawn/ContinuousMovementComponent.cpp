// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ContinuousMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/VRPawnInputConfig.h"
#include "Utility/VirtualRealityUtilities.h"

UContinuousMovementComponent::UContinuousMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, 80.0f);

	VRPawn = Cast<AVirtualRealityPawn>(GetOwner());
}

void UContinuousMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction){

	SetCapsuleColliderToUserSize();

	FVector PositionChange = GetPendingInputVector();

	if (NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		PositionChange.Z = 0.0f;
		ConsumeInputVector();
		AddInputVector(PositionChange);
	}
	
	if(NavigationMode == EVRNavigationModes::NAV_FLY || NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		MoveByGravityOrStepUp(DeltaTime);
		CheckForPhysWalkingCollision();

		if(CheckForVirtualMovCollision(PositionChange, DeltaTime))
		{
			ConsumeInputVector();
		}
	}

	if(NavigationMode == EVRNavigationModes::NAV_NONE)
	{
		ConsumeInputVector();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	LastHeadPosition = HeadComponent->GetComponentLocation();
}

void UContinuousMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetUpdatedComponent(VRPawn->GetRootComponent());
	SetHeadComponent(VRPawn->CapsuleRotationFix);

	SetupInputActions();
	
}

bool UContinuousMovementComponent::CheckForVirtualMovCollision(FVector PositionChange, float DeltaTime)
{
	FVector ProbePosition = PositionChange.GetSafeNormal() * GetMaxSpeed() * DeltaTime;
	FHitResult FHitResultVR;
	CapsuleColliderComponent->AddWorldOffset(ProbePosition, true, &FHitResultVR);
	if (FVector::Distance(FHitResultVR.Location, CapsuleColliderComponent->GetComponentLocation()) < CapsuleColliderComponent->GetScaledCapsuleRadius())
	{
		return true;
	}
	return false;
}

void UContinuousMovementComponent::SetHeadComponent(USceneComponent* NewHeadComponent)
{
	HeadComponent = NewHeadComponent;
	CapsuleColliderComponent->SetupAttachment(HeadComponent);
	const float HalfHeight = 80.0f; //this is just an initial value to look good in editor
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, HalfHeight);
	CapsuleColliderComponent->SetWorldLocation(FVector(0.0f, 0.0f,HalfHeight));
}

void UContinuousMovementComponent::SetCapsuleColliderToUserSize()
{
	float CharachterSize = abs(UpdatedComponent->GetComponentLocation().Z - HeadComponent->GetComponentLocation().Z);

	if (CharachterSize > MaxStepHeight)
	{
		float ColliderHeight = CharachterSize - MaxStepHeight;
		float ColliderHalfHeight = ColliderHeight / 2.0f;
		if (ColliderHalfHeight <= CapsuleRadius)
		{//Make the collider to a Sphere
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else
		{//Make the collider to a Capsule
			CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, ColliderHalfHeight);
		}

		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
		CapsuleColliderComponent->AddWorldOffset(FVector(0, 0, -ColliderHalfHeight));
		CapsuleColliderComponent->SetWorldRotation(FRotator(0, 0, 1));
	}
	else
	{
		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
		CapsuleColliderComponent->SetWorldRotation(FRotator(0, 0, 1));
	}
}

void UContinuousMovementComponent::CheckForPhysWalkingCollision()
{
	FVector CurrentHeadPosition = HeadComponent->GetComponentLocation();
	FVector Direction = CurrentHeadPosition - LastHeadPosition;
	FHitResult FHitResultPhys;
	CapsuleColliderComponent->AddWorldOffset(Direction, true, &FHitResultPhys);

	if (FHitResultPhys.bBlockingHit)
	{
		UpdatedComponent->AddLocalOffset(FHitResultPhys.Normal*FHitResultPhys.PenetrationDepth);
	}
}

void UContinuousMovementComponent::MoveByGravityOrStepUp(float DeltaSeconds)
{
	FVector StartLineTraceUnderCollider = CapsuleColliderComponent->GetComponentLocation();
	StartLineTraceUnderCollider.Z -= CapsuleColliderComponent->GetScaledCapsuleHalfHeight();
	FHitResult HitDetailsMultiLineTrace = CreateMultiLineTrace(FVector(0, 0, -1), StartLineTraceUnderCollider, CapsuleColliderComponent->GetScaledCapsuleRadius() / 4.0f, false);
	float DistanceDifference = abs(MaxStepHeight - HitDetailsMultiLineTrace.Distance);
	//Going up (in Fly and Walk Mode)
	if ((HitDetailsMultiLineTrace.bBlockingHit && HitDetailsMultiLineTrace.Distance < MaxStepHeight))
	{
		ShiftVertically(DistanceDifference, UpSteppingAcceleration, DeltaSeconds, 1);
	}
	//Gravity (only in Walk Mode)
	else if (NavigationMode==EVRNavigationModes::NAV_WALK && ((HitDetailsMultiLineTrace.bBlockingHit && HitDetailsMultiLineTrace.Distance > MaxStepHeight) || (HitDetailsMultiLineTrace.GetActor() == nullptr && HitDetailsMultiLineTrace.Distance != -1.0f)))
	{
		ShiftVertically(DistanceDifference, GravityAcceleration, DeltaSeconds, -1);
	}
}

void UContinuousMovementComponent::ShiftVertically(float DiffernceDistance, float VerticalAcceleration, float DeltaSeconds, int Direction)
{
	VerticalSpeed += VerticalAcceleration * DeltaSeconds;
	if (VerticalSpeed*DeltaSeconds < DiffernceDistance)
	{
		UpdatedComponent->AddLocalOffset(FVector(0.f, 0.f, Direction * VerticalSpeed * DeltaSeconds));
	}
	else
	{
		UpdatedComponent->AddLocalOffset(FVector(0.f, 0.f, Direction * DiffernceDistance));
		VerticalSpeed = 0;
	}
}

void UContinuousMovementComponent::SetupInputActions()
{
	// simple way of changing the handedness
	if(bMoveWithRightHand)
	{
		MovementHand = VRPawn->RightHand;
		RotationHand = VRPawn->LeftHand;
		IMCMovement = IMCMovementRight;
	} else
	{
		MovementHand = VRPawn->LeftHand;
		RotationHand = VRPawn->RightHand;
		IMCMovement = IMCMovementLeft;
	}
	
	const APlayerController* PlayerController = Cast<APlayerController>(VRPawn->GetController());
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!InputSubsystem)
	{
		UE_LOG(LogTemp,Error,TEXT("InputSubsystem IS NOT VALID"));
		return;
	}
	
	InputSubsystem->ClearAllMappings();

	// add Input Mapping context 
	InputSubsystem->AddMappingContext(IMCMovement,0);
	
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(VRPawn->InputComponent);
	if(!EI)
	{
		UE_LOG(LogTemp,Error,TEXT("Cannot cast Input Component to Enhanced Inpu Component in VRPawnMovement"));
		return;
	}
	
	// walking
	EI->BindAction(InputActions->Move, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnBeginMove);

	// turning
	if(bSnapTurn && !UVirtualRealityUtilities::IsDesktopMode())
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Started, this, &UContinuousMovementComponent::OnBeginSnapTurn);
	} else
	{
		EI->BindAction(InputActions->Turn, ETriggerEvent::Triggered, this, &UContinuousMovementComponent::OnBeginTurn);
	}
	
	// bind functions for desktop rotations only on holding down right mouse
	if (UVirtualRealityUtilities::IsDesktopMode())
	{
		APlayerController* PC = Cast<APlayerController>(VRPawn->GetController());
		if (PC)
		{
			PC->bShowMouseCursor = true; 
			PC->bEnableClickEvents = true; 
			PC->bEnableMouseOverEvents = true;
		}
		EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Started, this, &UContinuousMovementComponent::StartDesktopRotation);
		EI->BindAction(InputActions->DesktopRotation, ETriggerEvent::Completed, this, &UContinuousMovementComponent::EndDesktopRotation);
	}
}



FHitResult UContinuousMovementComponent::CreateLineTrace(FVector Direction, const FVector Start, bool Visibility)
{
	//Re-initialize hit info
	FHitResult HitDetails = FHitResult(ForceInit);

	FVector End = ((Direction * 1000.f) + Start);
	// additional trace parameters
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.bTraceComplex = true; //to use complex collision on whatever we interact with to provide better precision.
	TraceParams.bReturnPhysicalMaterial = true; //to provide details about the physical material, if one exists on the thing we hit, to come back in our hit result.

	if (Visibility)
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(HitDetails, Start, End, ECC_Visibility, TraceParams))
	{
		if (HitDetails.bBlockingHit)
		{
		}
	}
	return HitDetails;
}

FHitResult UContinuousMovementComponent::CreateMultiLineTrace(FVector Direction, const FVector Start, float Radius, bool Visibility)
{
	TArray<FVector> StartVectors;
	TArray<FHitResult> OutHits;
	FHitResult HitDetailsMultiLineTrace;
	HitDetailsMultiLineTrace.Distance = -1.0f;//(Distance=-1) not existing, but to know if this Variable not Initialized(when all Traces not compatible)

	StartVectors.Add(Start); //LineTraceCenter
	StartVectors.Add(Start + FVector(0, -Radius, 0)); //LineTraceLeft
	StartVectors.Add(Start + FVector(0, +Radius, 0)); //LineTraceRight
	StartVectors.Add(Start + FVector(+Radius, 0, 0)); //LineTraceFront
	StartVectors.Add(Start + FVector(-Radius, 0, 0)); //LineTraceBehind

	bool IsBlockingHitAndSameActor = true;
	bool IsAllNothingHiting = true;
	// loop through TArray
	for (FVector& Vector : StartVectors)
	{
		FHitResult OutHit = CreateLineTrace(Direction, Vector, Visibility);
		OutHits.Add(OutHit);
		IsBlockingHitAndSameActor &= (OutHit.GetActor() == OutHits[0].GetActor()); //If all Hiting the same Object, then you are (going up/down) or (walking)
		IsAllNothingHiting &= (OutHit.GetActor() == nullptr); //If all Hiting nothing, then you are falling
	}

	if (IsBlockingHitAndSameActor || IsAllNothingHiting)
		HitDetailsMultiLineTrace = OutHits[0];

	return HitDetailsMultiLineTrace;
}

void UContinuousMovementComponent::StartDesktopRotation()
{
	bApplyDesktopRotation = true;
}

void UContinuousMovementComponent::EndDesktopRotation()
{
	bApplyDesktopRotation = false;
}

void UContinuousMovementComponent::OnBeginMove(const FInputActionValue& Value)
{
	const FVector ForwardDir = UVirtualRealityUtilities::IsDesktopMode() ? VRPawn->Head->GetForwardVector() : MovementHand->GetForwardVector();
	const FVector RightDir = UVirtualRealityUtilities::IsDesktopMode() ? VRPawn->Head->GetRightVector() : MovementHand->GetRightVector();
	
	if (VRPawn->Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, VRPawn->Controller->GetControlRotation().Yaw, 0);
 
		// Forward/Backward direction
		if (MoveValue.X != 0.f)
		{
			VRPawn->AddMovementInput(ForwardDir, MoveValue.X);
		}
 
		// Right/Left direction
		if (MoveValue.Y != 0.f)
		{
			VRPawn->AddMovementInput(RightDir, MoveValue.Y);
		}
	}
}

void UContinuousMovementComponent::OnBeginTurn(const FInputActionValue& Value)
{
	if(UVirtualRealityUtilities::IsDesktopMode() && !bApplyDesktopRotation) return;
	if (VRPawn->Controller != nullptr)
	{
		const FVector2D TurnValue = Value.Get<FVector2D>();
 
		if (TurnValue.X != 0.f)
		{
			VRPawn->AddControllerYawInput(TurnRateFactor * TurnValue.X);
			if (UVirtualRealityUtilities::IsDesktopMode())
			{
				UpdateRightHandForDesktopInteraction();
			}
		}
 
		if (TurnValue.Y != 0.f)
		{
			if (UVirtualRealityUtilities::IsDesktopMode() && bApplyDesktopRotation)
			{
				VRPawn->AddControllerPitchInput(TurnRateFactor * TurnValue.Y);
				SetCameraOffset();
			}
		}
	}
}

void UContinuousMovementComponent::OnBeginSnapTurn(const FInputActionValue& Value)
{
	const FVector2D TurnValue = Value.Get<FVector2D>();
 
	if (TurnValue.X != 0.f)
	{
		VRPawn->AddControllerYawInput(SnapTurnAngle);
	}
}

void UContinuousMovementComponent::SetCameraOffset() const
{
	// this also incorporates the BaseEyeHeight, if set as static offset,
	// rotations are still around the center of the pawn (on the floor), so pitch rotations look weird
	FVector Location;
	FRotator Rotation;
	VRPawn->GetActorEyesViewPoint(Location, Rotation);
	VRPawn->CameraComponent->SetWorldLocationAndRotation(Location, Rotation);
}

void UContinuousMovementComponent::UpdateRightHandForDesktopInteraction()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FVector MouseLocation, MouseDirection;
		PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		FRotator HandOrientation = MouseDirection.ToOrientationRotator();
		VRPawn->RightHand->SetWorldRotation(HandOrientation);
	}
}

/*void UContinuousMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AVirtualRealityPawn, bMoveWithRightHand) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AVirtualRealityPawn, bSnapTurn))
	{
		// if we want to change input bindings, we need to setup the player input again to load a different mapping context,
		// or assign a different function to an input action.
		// This is automatically done by restarting the pawn(calling SetupPlayerInputComponent() directly results in crashes)
		// only do this in the editor
		#if WITH_EDITOR
			VRPawn->Restart();
#endif
	}
}*/