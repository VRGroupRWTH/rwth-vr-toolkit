// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/Interactables/IntenSelect/IntenSelectable.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableScoring.h"
#include "Interaction/Interactables/IntenSelect/IntenSelectableSinglePointScoring.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/MessageDialog.h"
#include "Pawn/IntenSelectComponent.h"

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
		if (!ScoringBehaviours.Num() == 0)
		{
			ShowErrorAndQuit(
				"Please assign the Scoring Behaviour manually when using more than one IntenSelectable Component!");
		}
	}
	else
	{
		InitDefaultBehaviourReferences();
	}
}

void UIntenSelectable::HandleOnSelectStartEvents(const UIntenSelectComponent* IntenSelect, const FHitResult& HitResult)
{
	for (const UHoverBehaviour* b : OnSelectBehaviours)
	{
		b->OnHoverStartEvent.Broadcast(IntenSelect, HitResult);
	}
}

void UIntenSelectable::HandleOnSelectEndEvents(const UIntenSelectComponent* IntenSelect)
{
	for (const UHoverBehaviour* b : OnSelectBehaviours)
	{
		b->OnHoverEndEvent.Broadcast(IntenSelect);
	}
}

void UIntenSelectable::HandleOnClickStartEvents(UIntenSelectComponent* IntenSelect)
{
	for (const UActionBehaviour* b : OnClickBehaviours)
	{
		FInputActionValue v{};
		const UInputAction* a{};
		b->OnActionBeginEvent.Broadcast(IntenSelect, a, v);
	}
}

void UIntenSelectable::HandleOnClickEndEvents(UIntenSelectComponent* IntenSelect, FInputActionValue& InputValue)
{
	for (const UActionBehaviour* b : OnClickBehaviours)
	{
		const UInputAction* a{};
		b->OnActionEndEvent.Broadcast(IntenSelect, a, InputValue);
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
	TInlineComponentArray<UHoverBehaviour*> AttachedSelectionBehaviours;
	GetOwner()->GetComponents(AttachedSelectionBehaviours, true);

	this->OnSelectBehaviours = AttachedSelectionBehaviours;

	// Clicking
	TInlineComponentArray<UActionBehaviour*> AttachedClickBehaviours;
	GetOwner()->GetComponents(AttachedClickBehaviours, true);

	this->OnClickBehaviours = AttachedClickBehaviours;
}

void UIntenSelectable::ShowErrorAndQuit(const FString& Message) const
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
#if WITH_EDITOR
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("RUNTIME ERROR")));
#endif
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
