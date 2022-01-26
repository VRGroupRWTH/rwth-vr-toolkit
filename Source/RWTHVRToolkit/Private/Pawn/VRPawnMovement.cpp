
#include "Pawn/VRPawnMovement.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

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
		// you are only allowed to move horizontally in NAV_WALK
		// everything else will be handled by stepping-up/gravity
		// so remove Z component for the input vector of the UFloatingPawnMovement
		PositionChange.Z = 0.0f;
		ConsumeInputVector();
		AddInputVector(PositionChange);
	}
	
	if(NavigationMode == EVRNavigationModes::NAV_FLY || NavigationMode == EVRNavigationModes::NAV_WALK)
	{
		CheckForPhysWalkingCollision();
		if(CheckForVirtualSteerCollision(PositionChange, DeltaTime))
		{
			// if we would move into something if we apply this input (estimating distance by max speed)
			// we consume the input so it is not applied
			ConsumeInputVector();
		}

		// so we add stepping-up (for both walk and fly)
		// and gravity for walking only
		MoveByGravityOrStepUp(DeltaTime);
	}

	if(NavigationMode == EVRNavigationModes::NAV_NONE)
	{
		//just remove whatever input is there
		ConsumeInputVector();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	LastHeadPosition = HeadComponent->GetComponentLocation();
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
		{//Make the collider to a Sphere
			CapsuleColliderComponent->SetCapsuleSize(ColliderHalfHeight, ColliderHalfHeight);
		}
		else
		{//Make the collider to a Capsule
			CapsuleColliderComponent->SetCapsuleSize(CapsuleRadius, ColliderHalfHeight);
		}

		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
		CapsuleColliderComponent->AddWorldOffset(FVector(0, 0, -ColliderHalfHeight));
	}
	else
	{
		CapsuleColliderComponent->SetWorldLocation(HeadComponent->GetComponentLocation());
	}
	CapsuleColliderComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void UVRPawnMovement::CheckForPhysWalkingCollision()
{
	const FHitResult HitResult = CreateCapsuleTrace(LastHeadPosition, HeadComponent->GetComponentLocation(), false);

	//if this was not possible move it the entire pawn to avoid the head collision
	if (HitResult.bBlockingHit)
	{
		UpdatedComponent->AddLocalOffset(HitResult.Normal*HitResult.PenetrationDepth);
	}
}

bool UVRPawnMovement::CheckForVirtualSteerCollision(FVector PositionChange, float DeltaTime)
{
	FVector ProbePosition = PositionChange.GetSafeNormal() * GetMaxSpeed() * DeltaTime;
	const FHitResult HitResult = CreateCapsuleTrace(HeadComponent->GetComponentLocation(), ProbePosition, false);
	if (HitResult.bBlockingHit)
	{
		return true;
	}
	return false;
}

void UVRPawnMovement::MoveByGravityOrStepUp(float DeltaSeconds)
{
	const FVector DownTraceStart = CapsuleColliderComponent->GetComponentLocation();
	const float DownTraceDist = 1000.0f;
	const FVector DownTraceDir = FVector(0,0,-1);
	const FVector DownTraceEnd = DownTraceStart + DownTraceDist * DownTraceDir;

	const FHitResult DownTraceHitResult = CreateCapsuleTrace(DownTraceStart, DownTraceEnd, true);
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

	//Gravity (only in Walk Mode)
	if (!DownTraceHitResult.bBlockingHit || HeightDifference<0.0f)
	{
		ShiftVertically(HeightDifference, GravityAcceleration, DeltaSeconds);
	}
}

void UVRPawnMovement::ShiftVertically(float Distance, float VerticalAcceleration, float DeltaSeconds)
{
	VerticalSpeed += VerticalAcceleration * DeltaSeconds;
	if (abs(VerticalSpeed*DeltaSeconds) < abs(Distance))
	{
		UpdatedComponent->AddLocalOffset(FVector(0.f, 0.f,  VerticalSpeed * DeltaSeconds));
	}
	else
	{
		UpdatedComponent->AddLocalOffset(FVector(0.f, 0.f,  Distance));
		VerticalSpeed = 0;
	}
}

FHitResult UVRPawnMovement::CreateCapsuleTrace(const FVector Start, FVector End, bool DrawDebug) const
{
	const EDrawDebugTrace::Type DrawType = DrawDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;

	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), Start, End, CapsuleColliderComponent->GetScaledCapsuleRadius(), CapsuleColliderComponent->GetScaledCapsuleHalfHeight() ,UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, {}, DrawType, Hit, true);
	return Hit;
}
