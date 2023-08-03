// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/IntenSelectable.h"

#include "Interaction/ClickBehaviour.h"
#include "Interaction/IntenSelectableSinglePointScoring.h"
#include "Interaction/SelectionBehaviour.h"
#include "Misc/MessageDialog.h"

UIntenSelectable::UIntenSelectable()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FVector, float> UIntenSelectable::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime) const
{
	check(ScoringBehaviour)
	return ScoringBehaviour->GetBestPointScorePair(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, LastValue, DeltaTime);
}

void UIntenSelectable::HandleOnSelectStartEvents(const UIntenSelectComponent* IntenSelect, const FVector& Point)
{
	for(const USelectionBehaviour* b : OnSelectBehaviours)
	{
		b->OnSelectStartEvent.Broadcast(IntenSelect, Point);
	}
}

void UIntenSelectable::HandleOnSelectEndEvents(const UIntenSelectComponent* IntenSelect)
{
	for(const USelectionBehaviour* b : OnSelectBehaviours)
	{
		b->OnSelectEndEvent.Broadcast(IntenSelect);
	}
}

void UIntenSelectable::HandleOnClickStartEvents(const UIntenSelectComponent* IntenSelect, const FVector& Point)
{
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		b->OnClickStartEvent.Broadcast(IntenSelect, Point);
	}
}

void UIntenSelectable::HandleOnClickEndEvents(const UIntenSelectComponent* IntenSelect)
{
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		b->OnClickEndEvent.Broadcast(IntenSelect);
	}
}

void UIntenSelectable::InitDefaultBehaviourReferences()
{
	//Scoring
	UIntenSelectableScoring* AttachedScoring = Cast<UIntenSelectableScoring>(GetOwner()->GetComponentByClass(UIntenSelectableScoring::StaticClass()));
	if(AttachedScoring)
	{
		ScoringBehaviour = AttachedScoring;
	}else
	{
		ScoringBehaviour = NewObject<UIntenSelectableSinglePointScoring>(this, UIntenSelectableSinglePointScoring::StaticClass(), "Default Scoring");
		ScoringBehaviour->SetWorldLocation(GetOwner()->GetActorLocation());
		ScoringBehaviour->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	//Selecting
	TInlineComponentArray<USelectionBehaviour*> AttachedSelectionBehaviours;
	GetOwner()->GetComponents(AttachedSelectionBehaviours, true);

	this->OnSelectBehaviours = AttachedSelectionBehaviours;

	//Clicking
	TInlineComponentArray<UClickBehaviour*> AttachedClickBehaviours;
	GetOwner()->GetComponents(AttachedClickBehaviours, true);

	this->OnClickBehaviours = AttachedClickBehaviours;
}

void UIntenSelectable::ShowErrorAndQuit(const FString& Message) const
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
#if WITH_EDITOR
	const FText Title = FText::FromString(FString("RUNTIME ERROR"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), &Title);
#endif
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UIntenSelectable::BeginPlay()
{
	Super::BeginPlay();

	TInlineComponentArray<UIntenSelectable*> AttachedIntenSelectables;
	GetOwner()->GetComponents(AttachedIntenSelectables, false);
		
	if(AttachedIntenSelectables.Num() > 1)
	{
		if(!ScoringBehaviour)
		{
			ShowErrorAndQuit("Please assign the Scoring Behaviour manually when using more than one IntenSelectable Component!");
		}
	}else
	{
		InitDefaultBehaviourReferences();	
	}	
}
