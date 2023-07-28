
#include "Pawn/VRPawnMovement.h"
#include "DrawDebugHelpers.h"

UVRPawnMovement::UVRPawnMovement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, 80.0f);
}

void UVRPawnMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction){

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

bool UVRPawnMovement::CheckForVirtualMovCollision(FVector PositionChange, float DeltaTime)
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

void UVRPawnMovement::SetHeadComponent(USceneComponent* NewHeadComponent)
{
	HeadComponent = NewHeadComponent;
	CapsuleColliderComponent->SetupAttachment(HeadComponent);
	const float HalfHeight = 80.0f; //this is just an initial value to look good in editor
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, HalfHeight);
	CapsuleColliderComponent->SetWorldLocation(FVector(0.0f, 0.0f,HalfHeight));
}

void UVRPawnMovement::SetCapsuleColliderToUserSize()
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

void UVRPawnMovement::CheckForPhysWalkingCollision()
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

void UVRPawnMovement::MoveByGravityOrStepUp(float DeltaSeconds)
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

void UVRPawnMovement::ShiftVertically(float DiffernceDistance, float VerticalAcceleration, float DeltaSeconds, int Direction)
{
	VerticalSpeed += VerticalAcceleration * DeltaSeconds;
	if (VerticalSpeed*DeltaSeconds < DiffernceDistance)
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f, Direction * VerticalSpeed * DeltaSeconds));
	}
	else
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f, Direction * DiffernceDistance));
		VerticalSpeed = 0;
	}
}

FHitResult UVRPawnMovement::CreateLineTrace(FVector Direction, const FVector Start, bool Visibility)
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

FHitResult UVRPawnMovement::CreateMultiLineTrace(FVector Direction, const FVector Start, float Radius, bool Visibility)
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