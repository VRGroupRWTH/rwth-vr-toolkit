#include "Interaction/Interactables/IntenSelect/IntenSelectable.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableSinglePointScoring.h"
#include "Misc/MessageDialog.h"
#include "Pawn/IntenSelectComponent.h"
#include "Utility/RWTHVRUtilities.h"

UIntenSelectable::UIntenSelectable() { PrimaryComponentTick.bCanEverTick = true; }

TPair<FHitResult, float> UIntenSelectable::GetBestPointScorePair(const FVector& ConeOrigin,
																 const FVector& ConeForwardDirection,
																 const float ConeBackwardShiftDistance,
																 const float ConeAngle, const float LastValue,
																 const float DeltaTime) const
{
	checkf(ScoringBehaviours.Num() > 0, TEXT("%s"), *GetOwner()->GetName());

	float MaxScore = -1;
	FHitResult MaxResult;

	for (UIntenSelectableScoring* s : ScoringBehaviours)
	{
		const TPair<FHitResult, float> Score_Pair = s->GetBestPointScorePair(
			ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, LastValue, DeltaTime);

		if (Score_Pair.Value >= MaxScore)
		{
			MaxResult = Score_Pair.Key;
			MaxScore = Score_Pair.Value;
		}
	}
	return TPair<FHitResult, float>{MaxResult, MaxScore};
}

void UIntenSelectable::BeginPlay()
{
	Super::BeginPlay();

	TInlineComponentArray<UIntenSelectable*> AttachedIntenSelectables;
	GetOwner()->GetComponents(AttachedIntenSelectables, false);

	if (AttachedIntenSelectables.Num() > 1)
	{
		if (ScoringBehaviours.Num() == 0)
		{
			URWTHVRUtilities::ShowErrorAndQuit(
				"Please assign the Scoring Behaviour manually when using more than one IntenSelectable Component!",
				false, this);
		}
	}
	else
	{
		InitDefaultBehaviourReferences();
	}
}

void UIntenSelectable::HandleOnHoverStartEvents(const UIntenSelectComponent* IntenSelect, const FHitResult& HitResult)
{
	for (const UHoverBehaviour* CurrentHoverBehaviour : OnHoverBehaviours)
	{
		CurrentHoverBehaviour->OnHoverStartEvent.Broadcast(IntenSelect, HitResult);
	}
}

void UIntenSelectable::HandleOnHoverEndEvents(const UIntenSelectComponent* IntenSelect)
{
	for (const UHoverBehaviour* CurrentHoverBehaviour : OnHoverBehaviours)
	{
		CurrentHoverBehaviour->OnHoverEndEvent.Broadcast(IntenSelect);
	}
}

void UIntenSelectable::HandleOnActionStartEvents(UIntenSelectComponent* IntenSelect)
{
	for (const UActionBehaviour* CurrentActionBehaviour : OnActionBehaviours)
	{
		FInputActionValue EmptyInputActionValue{};
		const UInputAction* EmptyInputAction{};
		CurrentActionBehaviour->OnActionBeginEvent.Broadcast(IntenSelect, EmptyInputAction, EmptyInputActionValue);
	}
}

void UIntenSelectable::HandleOnActionEndEvents(UIntenSelectComponent* IntenSelect, FInputActionValue& InputValue)
{
	for (const UActionBehaviour* CurrentActionBehaviour : OnActionBehaviours)
	{
		const UInputAction* EmptyInputActionValue{};
		CurrentActionBehaviour->OnActionEndEvent.Broadcast(IntenSelect, EmptyInputActionValue, InputValue);
	}
}

void UIntenSelectable::InitDefaultBehaviourReferences()
{
	// Scoring

	for (TSet<UActorComponent*> AllComponents = GetOwner()->GetComponents(); UActorComponent * c : AllComponents)
	{
		if (UIntenSelectableScoring* TryToGetScoring = Cast<UIntenSelectableScoring>(c))
		{
			ScoringBehaviours.Add(TryToGetScoring);
		}
	}

	if (ScoringBehaviours.Num() == 0)
	{
		const auto InitScoringBehaviour = NewObject<UIntenSelectableSinglePointScoring>(
			this, UIntenSelectableSinglePointScoring::StaticClass(), "Default Scoring");
		InitScoringBehaviour->SetWorldLocation(GetOwner()->GetActorLocation());
		InitScoringBehaviour->AttachToComponent(GetOwner()->GetRootComponent(),
												FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		ScoringBehaviours.Add(InitScoringBehaviour);
	}

	// Selecting
	TInlineComponentArray<UHoverBehaviour*> AttachedHoverBehaviours;
	GetOwner()->GetComponents(AttachedHoverBehaviours, true);

	OnHoverBehaviours = AttachedHoverBehaviours;

	// Clicking
	TInlineComponentArray<UActionBehaviour*> AttachedClickBehaviours;
	GetOwner()->GetComponents(AttachedClickBehaviours, true);

	OnActionBehaviours = AttachedClickBehaviours;
}