#include "Pawn/Navigation/CollisionHandlingMovement.h"

#include "Kismet/KismetSystemLibrary.h"

UCollisionHandlingMovement::UCollisionHandlingMovement(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// the capsule is used to store the players size and position, e.g., for other interactions and as starting point
	// for the capsule trace (which however not use the capsule component directly)
	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,
															ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, 80.0f);

	// set some defaults for the UFloatingPawnMovement component, which are more reasonable for usage in VR
	MaxSpeed = 300.f;
	Acceleration = 800.f;
	Deceleration = 2000.f;
}

void UCollisionHandlingMovement::BeginPlay()
{
	Super::BeginPlay();
	LastSteeringCollisionVector = FVector(0, 0, 0);
	LastCollisionFreeCapsulePosition.Reset();

	ActorsToIgnore = {GetOwner()};
}


void UCollisionHandlingMovement::TickComponent(float DeltaTime, enum ELevelTick TickType,
											   FActorComponentTickFunction* ThisTickFunction)
{
	SetCapsuleColliderToUserSize();

	FVector InputVector = GetPendingInputVector();

	if (NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		// you are only allowed to move horizontally in NAV_WALK
		// everything else will be handled by stepping-up/gravity
		// so remove Z component for the input vector of the UFloatingPawnMovement
		InputVector.Z = 0.0f;
		ConsumeInputVector();
		AddInputVector(InputVector);
	}


	if (NavigationMode == EVRNavigationModes::NAV_FLY || NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		// if me managed to get into a collision revert the movement since last Tick
		CheckAndRevertCollisionSinceLastTick();
		// check whether we are still in collision e.g. if an object has moved and got us into collision
		MoveOutOfNewDynamicCollisions();

		if (InputVector.Size() > 0.001)
		{
			const FVector SafeSteeringInput = GetCollisionSafeVirtualSteeringVec(InputVector, DeltaTime);
			if (SafeSteeringInput != InputVector)
			{
				// if we would move into something if we apply this input (estimating distance by max speed)
				// we only apply its perpendicular part (unless it is facing away from the collision)
				ConsumeInputVector();
				AddInputVector(SafeSteeringInput);
			}
		}

		// in case we are in a collision and collision checks are temporarily deactivated, we only allow physical
		// movement without any checks, otherwise check collision during physical movement
		if (bCollisionChecksTemporarilyDeactivated)
		{
			ConsumeInputVector();
		}
		else
		{
			// so we add stepping-up (for both walk and fly)
			// and gravity for walking only
			MoveByGravityOrStepUp(DeltaTime);

			// if we physically (in the tracking space) walked into something, move the world away (by moving the pawn)
			CheckForPhysWalkingCollision();
		}
	}

	if (NavigationMode == EVRNavigationModes::NAV_NONE)
	{
		// just remove whatever input is there
		ConsumeInputVector();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCollisionHandlingMovement::SetHeadComponent(USceneComponent* NewHeadComponent)
{
	HeadComponent = NewHeadComponent;
	CapsuleColliderComponent->SetupAttachment(HeadComponent);
	const float HalfHeight = 80.0f; // this is just an initial value to look good in editor
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, HalfHeight);
	CapsuleColliderComponent->SetWorldLocation(FVector(0.0f, 0.0f, -HalfHeight));
}

void UCollisionHandlingMovement::SetCapsuleColliderToUserSize() const
{
	// the collider should be placed
	//	between head and floor + MaxStepHeight
	//             head
	//            /    \
	//           /      \
	//          |        |
	//          |        |
	//          |collider|
	//          |        |
	//          |        |
	//           \      /
	//            \ __ /
	//              |
	//         MaxStepHeight
	//              |
	// floor: ______________

	const float UserSize = HeadComponent->GetComponentLocation().Z - UpdatedComponent->GetComponentLocation().Z;

	if (UserSize > MaxStepHeight)
	{
		const float ColliderHeight = UserSize - MaxStepHeight;
		const float ColliderHalfHeight = ColliderHeight / 2.0f;
		if (ColliderHalfHeight <= CapsuleRadius)
		{
			// the capsule will actually be compressed to a sphere
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else
		{
			CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, ColliderHalfHeight);
		}

		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation() -
												   FVector(0, 0, ColliderHalfHeight));
	}
	else
	{
		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
	}

	CapsuleColliderComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void UCollisionHandlingMovement::CheckAndRevertCollisionSinceLastTick()
{
	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();

	if (!LastCollisionFreeCapsulePosition.IsSet())
	{
		// we cannot revert anyways so only check if the current position is collision free
		if (!CreateCapsuleTrace(CapsuleLocation, CapsuleLocation).bBlockingHit)
		{
			LastCollisionFreeCapsulePosition = CapsuleLocation;
			bCollisionChecksTemporarilyDeactivated = false;
		}
		return;
	}

	// check whether we are in a collision at the current position
	if (CreateCapsuleTrace(CapsuleLocation, CapsuleLocation).bBlockingHit)
	{
		// if so move back to last position, but only if that position is still collision free
		// since the user might have moveed physically in between
		FVector LastCapsulePos = LastCollisionFreeCapsulePosition.GetValue();
		if (!CreateCapsuleTrace(LastCapsulePos, LastCapsulePos).bBlockingHit)
		{
			UpdatedComponent->AddWorldOffset(LastCapsulePos - CapsuleLocation);
		}
		else
		{
			bCollisionChecksTemporarilyDeactivated = true;
			LastCollisionFreeCapsulePosition.Reset();
		}
	}
	else
	{
		LastCollisionFreeCapsulePosition = CapsuleLocation;
	}
}

void UCollisionHandlingMovement::MoveOutOfNewDynamicCollisions()
{
	TOptional<FVector> ResolveDirectionOptional = GetOverlapResolveDirection();

	if (ResolveDirectionOptional.IsSet())
	{
		FVector ResolveDirection = 1.5f * ResolveDirectionOptional.GetValue(); // scale it up for security distance
		UpdatedComponent->AddWorldOffset(ResolveDirection);

		// check whether this helped in resolving the collision, and if not deactivate collision checks temporarily
		if (CreateCapsuleTrace(UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentLocation())
				.bBlockingHit)
		{
			bCollisionChecksTemporarilyDeactivated = true;
		}

		// invalidate the last collision-free position, since apparently something changed so we got into this collision
		LastCollisionFreeCapsulePosition.Reset();
	}
}

void UCollisionHandlingMovement::CheckForPhysWalkingCollision()
{
	if (!LastCollisionFreeCapsulePosition.IsSet())
	{
		// we don't know any old collision-free location, so do nothing here
		return;
	}

	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();
	const FHitResult HitResult = CreateCapsuleTrace(LastCollisionFreeCapsulePosition.GetValue(), CapsuleLocation);

	// if this was not possible move the entire pawn away to avoid the head collision
	if (HitResult.bBlockingHit)
	{
		const FVector MoveOutVector = HitResult.Location - CapsuleLocation;
		// move it out a bit farther, to avoid getting stuck situations
		UpdatedComponent->AddWorldOffset(1.2f * MoveOutVector);
	}
}

FVector UCollisionHandlingMovement::GetCollisionSafeVirtualSteeringVec(FVector InputVector, float DeltaTime)
{
	// if we were in a collision in the last step already (so no LastCollisionFreeCapsulePosition is set)
	// we allow movement to resole this collision (otherwise you wold be stuck forever)
	if (!LastCollisionFreeCapsulePosition.IsSet())
	{
		return InputVector;
	}


	const float SafetyFactor = 3.0f; // so we detect collision a bit earlier
	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();
	FVector ProbePosition = SafetyFactor * InputVector.GetSafeNormal() * GetMaxSpeed() * DeltaTime + CapsuleLocation;
	const FHitResult TraceResult = CreateCapsuleTrace(CapsuleLocation, ProbePosition);
	if (!TraceResult.bBlockingHit)
	{
		// everything is fine, use that vector
		return InputVector;
	}

	// otherwise remove the component of that vector that goes towards the collision
	FVector CollisionVector = TraceResult.Location - CapsuleLocation;

	// sometimes (if by chance we already moved into collision entirely CollisionVector is 0
	if (!CollisionVector.Normalize())
	{
		// then we probably start already in collision, so we use the last one
		CollisionVector = LastSteeringCollisionVector;
	}
	else
	{
		LastSteeringCollisionVector = CollisionVector;
	}

	FVector SafeInput = InputVector;
	const float DotProduct = FVector::DotProduct(InputVector, CollisionVector);
	if (DotProduct > 0.0f)
	{
		// only keep perpendicular part of the input vector (remove anything towards hit)
		SafeInput -= DotProduct * CollisionVector;
	}
	return SafeInput;
}

void UCollisionHandlingMovement::MoveByGravityOrStepUp(float DeltaSeconds)
{
	const FVector DownTraceStart = CapsuleColliderComponent->GetComponentLocation();
	const float DownTraceDist = MaxFallingDepth < 0.0f ? 1000.0f : MaxFallingDepth;
	const FVector DownTraceDir = FVector(0, 0, -1);
	const FVector DownTraceEnd = DownTraceStart + DownTraceDist * DownTraceDir;

	const FHitResult DownTraceHitResult = CreateCapsuleTrace(DownTraceStart, DownTraceEnd);
	float HeightDifference = 0.0f;

	if (DownTraceHitResult.bBlockingHit)
	{
		HeightDifference = DownTraceHitResult.ImpactPoint.Z - UpdatedComponent->GetComponentLocation().Z;
		// so for HeightDifference>0, we have to move the pawn up; for HeightDifference<0 we have to move it down
	}

	// Going up (in Fly and Walk Mode)
	if (HeightDifference > 0.0f && HeightDifference <= MaxStepHeight)
	{
		ShiftVertically(HeightDifference, UpSteppingAcceleration, DeltaSeconds);
	}

	if (NavigationMode != EVRNavigationModes::NAV_WALK)
	{
		return;
	}

	if (!DownTraceHitResult.bBlockingHit && MaxFallingDepth < 0.0f)
	{
		HeightDifference = -1000.0f; // just fall
	}

	// Gravity (only in Walk Mode)
	if (HeightDifference < 0.0f)
	{
		ShiftVertically(HeightDifference, GravityAcceleration, DeltaSeconds);
	}
}

void UCollisionHandlingMovement::ShiftVertically(float Distance, float VerticalAcceleration, float DeltaSeconds)
{
	VerticalSpeed += VerticalAcceleration * DeltaSeconds;
	if (abs(VerticalSpeed * DeltaSeconds) < abs(Distance))
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f, VerticalSpeed * DeltaSeconds));
	}
	else
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f, Distance));
		VerticalSpeed = 0;
	}
}

FHitResult UCollisionHandlingMovement::CreateCapsuleTrace(const FVector& Start, const FVector& End,
														  const bool DrawDebug) const
{
	const EDrawDebugTrace::Type DrawType = DrawDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;

	// UE_LOG(LogTemp, Warning, TEXT("Capsule from %s to %s"), *Start.ToString(), *End.ToString())

	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), Start, End, CapsuleColliderComponent->GetScaledCapsuleRadius(),
											 CapsuleColliderComponent->GetScaledCapsuleHalfHeight(),
											 UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true,
											 ActorsToIgnore, DrawType, Hit, true);
	return Hit;
}

TOptional<FVector> UCollisionHandlingMovement::GetOverlapResolveDirection() const
{
	TArray<UPrimitiveComponent*> OverlappingComponents;
	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery_MAX);

	UKismetSystemLibrary::CapsuleOverlapComponents(GetWorld(), CapsuleColliderComponent->GetComponentLocation(),
												   CapsuleColliderComponent->GetScaledCapsuleRadius(),
												   CapsuleColliderComponent->GetScaledCapsuleHalfHeight(),
												   traceObjectTypes, nullptr, ActorsToIgnore, OverlappingComponents);

	TOptional<FVector> ResolveVector;
	// check what to do to move out of these collisions (or nothing if none is there)
	// we just add the penetrations so in very unfortunate conditions this can become problematic/blocking but for now
	// and our regular use cases this works
	for (const UPrimitiveComponent* OverlappingComp : OverlappingComponents)
	{
		FHitResult Hit = CreateCapsuleTrace(CapsuleColliderComponent->GetComponentLocation(),
											OverlappingComp->GetComponentLocation(), false);

		if (Hit.bBlockingHit)
		{
			FVector Change = Hit.ImpactNormal * Hit.PenetrationDepth;
			ResolveVector = ResolveVector.IsSet() ? ResolveVector.GetValue() + Change : Change;
		}
	}

	return ResolveVector;
}
