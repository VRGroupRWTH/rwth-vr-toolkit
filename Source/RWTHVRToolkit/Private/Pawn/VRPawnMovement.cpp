
#include "Pawn/VRPawnMovement.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UVRPawnMovement::UVRPawnMovement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// the capsule is used to store the players size and position, e.g., for other interactions and as starting point
	// for the capsule trace (which however not use the capsule component directly)
	CapsuleColliderComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, 80.0f);

	//set some defaults for the UFloatingPawnMovement component, which are more reasonable for usage in VR
	MaxSpeed = 300.f;
	Acceleration = 800.f;
	Deceleration = 2000.f;
}

void UVRPawnMovement::BeginPlay()
{
	Super::BeginPlay();
	LastCapsulePosition.Reset();
	LastSteeringCollisionVector = FVector(0,0,0);
}


void UVRPawnMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction){

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

	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();
	if(bDeactivatedWhileInCollision && !CreateCapsuleTrace(CapsuleLocation, CapsuleLocation).bBlockingHit)
	{
		bDeactivatedWhileInCollision=false;
	}
	
	if(NavigationMode == EVRNavigationModes::NAV_FLY || NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		if(InputVector.Size() > 0.001){
			const FVector SafeSteeringInput = GetCollisionSafeVirtualSteeringVec(InputVector, DeltaTime);
			if(SafeSteeringInput != InputVector)
			{
				// if we would move into something if we apply this input (estimating distance by max speed)
				// we only apply its perpendicular part (unless it is facing away from the collision)
				ConsumeInputVector();
				AddInputVector(SafeSteeringInput);
			}
		}

		// so we add stepping-up (for both walk and fly)
		// and gravity for walking only
		MoveByGravityOrStepUp(DeltaTime);

		//if we physically (in the tracking space) walked into something, move the world away (by moving the pawn)
		if(!bDeactivatedWhileInCollision) CheckForPhysWalkingCollision();
	}

	if(NavigationMode == EVRNavigationModes::NAV_NONE)
	{
		//just remove whatever input is there
		ConsumeInputVector();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVRPawnMovement::SetHeadComponent(USceneComponent* NewHeadComponent)
{
	HeadComponent = NewHeadComponent;
	CapsuleColliderComponent->SetupAttachment(HeadComponent);
	const float HalfHeight = 80.0f; //this is just an initial value to look good in editor
	CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, HalfHeight);
	CapsuleColliderComponent->SetWorldLocation(FVector(0.0f, 0.0f,-HalfHeight));
}

void UVRPawnMovement::SetCapsuleColliderToUserSize()
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
			//the capsule will actually be compressed to a sphere
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else
		{
			CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, ColliderHalfHeight);
		}

		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation() - FVector(0, 0, ColliderHalfHeight));
	}
	else
	{
		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
	}

	CapsuleColliderComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void UVRPawnMovement::CheckForPhysWalkingCollision()
{
	if(!LastCapsulePosition.IsSet())
	{
		//not yet set, so do nothing than setting it
		LastCapsulePosition = CapsuleColliderComponent->GetComponentLocation();
		return;
	}

	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();
	const FHitResult HitResult = CreateCapsuleTrace(LastCapsulePosition.GetValue(), CapsuleLocation);

	//if this was not possible move the entire pawn away to avoid the head collision
	if (HitResult.bBlockingHit)
	{
		const FVector MoveOutVector = HitResult.Location-CapsuleLocation;
		//move it out twice as far, to avoid getting stuck situations
		UpdatedComponent->AddWorldOffset(2*MoveOutVector);
	}


	//only update if not in collision
	if(!CreateCapsuleTrace(CapsuleLocation, CapsuleLocation).bBlockingHit)
	{
		LastCapsulePosition = CapsuleColliderComponent->GetComponentLocation();;
	}
	else{
		//we are still in collision, so deactivate collision handling until this stopped
		bDeactivatedWhileInCollision=true;
		LastCapsulePosition.Reset();
	}
}

FVector UVRPawnMovement::GetCollisionSafeVirtualSteeringVec(FVector InputVector, float DeltaTime)
{

	const float SafetyFactor = 3.0f; //so we detect collision a bit earlier
	const FVector CapsuleLocation = CapsuleColliderComponent->GetComponentLocation();
	FVector ProbePosition = SafetyFactor * InputVector.GetSafeNormal() * GetMaxSpeed() * DeltaTime + CapsuleLocation;
	const FHitResult TraceResult = CreateCapsuleTrace(CapsuleLocation, ProbePosition);
	if (!TraceResult.bBlockingHit)
	{
		//everything is fine, use that vector
		return InputVector;
	}

	//otherwise remove the component of that vector that goes towards the collision
	FVector CollisionVector = TraceResult.Location - CapsuleLocation;

	//sometimes (if by chance we already moved into collision entirely CollisionVector is 0
	if(! CollisionVector.Normalize())
	{
		//then we probably start already in collision, so we use the last one
		CollisionVector = LastSteeringCollisionVector;
	}
	else
	{
		LastSteeringCollisionVector = CollisionVector;
	}

	FVector SafeInput = InputVector;
	const float DotProduct =  FVector::DotProduct(InputVector, CollisionVector);
	if(DotProduct>0.0f)
	{
		// only keep perpendicular part of the input vector (remove anything towards hit)
		SafeInput -= DotProduct * CollisionVector;
	}
	return SafeInput;
}

void UVRPawnMovement::MoveByGravityOrStepUp(float DeltaSeconds)
{
	const FVector DownTraceStart = CapsuleColliderComponent->GetComponentLocation();
	const float DownTraceDist = MaxFallingDepth < 0.0f ? 1000.0f : MaxFallingDepth;
	const FVector DownTraceDir = FVector(0,0,-1);
	const FVector DownTraceEnd = DownTraceStart + DownTraceDist * DownTraceDir;

	const FHitResult DownTraceHitResult = CreateCapsuleTrace(DownTraceStart, DownTraceEnd);
	float HeightDifference = 0.0f;

	if(DownTraceHitResult.bBlockingHit)
	{
		HeightDifference = DownTraceHitResult.ImpactPoint.Z - UpdatedComponent->GetComponentLocation().Z;
		//so for HeightDifference>0, we have to move the pawn up; for HeightDifference<0 we have to move it down
	}

	//Going up (in Fly and Walk Mode)
	if (HeightDifference>0.0f && HeightDifference<=MaxStepHeight)
	{
		ShiftVertically(HeightDifference, UpSteppingAcceleration, DeltaSeconds);
	}

	if(NavigationMode!=EVRNavigationModes::NAV_WALK)
	{
		return;
	}

	if(!DownTraceHitResult.bBlockingHit && MaxFallingDepth<0.0f)
	{
		HeightDifference = -1000.0f; //just fall
	}

	//Gravity (only in Walk Mode)
	if (HeightDifference<0.0f)
	{
		ShiftVertically(HeightDifference, GravityAcceleration, DeltaSeconds);
	}
}

void UVRPawnMovement::ShiftVertically(float Distance, float VerticalAcceleration, float DeltaSeconds)
{
	VerticalSpeed += VerticalAcceleration * DeltaSeconds;
	if (abs(VerticalSpeed*DeltaSeconds) < abs(Distance))
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f,  VerticalSpeed * DeltaSeconds));
	}
	else
	{
		UpdatedComponent->AddWorldOffset(FVector(0.f, 0.f,  Distance));
		VerticalSpeed = 0;
	}
}

FHitResult UVRPawnMovement::CreateCapsuleTrace(const FVector Start, FVector End, bool DrawDebug)
{
	const EDrawDebugTrace::Type DrawType = DrawDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;
	if(ActorsToIgnore.Num()==0){
		ActorsToIgnore.Add(GetOwner());
	}

	//UE_LOG(LogTemp, Warning, TEXT("Capsule from %s to %s"), *Start.ToString(), *End.ToString())

	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), Start, End, CapsuleColliderComponent->GetScaledCapsuleRadius(), CapsuleColliderComponent->GetScaledCapsuleHalfHeight(), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, ActorsToIgnore, DrawType, Hit, true);
	return Hit;
}
