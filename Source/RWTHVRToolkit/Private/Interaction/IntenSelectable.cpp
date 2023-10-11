// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/IntenSelectable.h"
#include "Interaction/ClickBehaviour.h"
#include "Interaction/IntenSelectableSinglePointScoring.h"
#include "Interaction/HoverBehaviour.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/MessageDialog.h"
#include "Pawn/IntenSelectComponent.h"

UIntenSelectable::UIntenSelectable()
{
	PrimaryComponentTick.bCanEverTick = true;
}

TPair<FHitResult, float> UIntenSelectable::GetBestPointScorePair(const FVector& ConeOrigin,
	const FVector& ConeForwardDirection, const float ConeBackwardShiftDistance, const float ConeAngle,
	const float LastValue, const float DeltaTime) const
{
	checkf(ScoringBehaviour,TEXT("%s"),*GetOwner()->GetName())
	return ScoringBehaviour->GetBestPointScorePair(ConeOrigin, ConeForwardDirection, ConeBackwardShiftDistance, ConeAngle, LastValue, DeltaTime);
}

void UIntenSelectable::HandleOnSelectStartEvents(const UIntenSelectComponent* IntenSelect, const FHitResult& HitResult)
{
	for(const UHoverBehaviour* b : OnSelectBehaviours)
	{
		b->OnHoverStartEvent.Broadcast(IntenSelect, HitResult);
	}
}

void UIntenSelectable::HandleOnSelectEndEvents(const UIntenSelectComponent* IntenSelect)
{
	for(const UHoverBehaviour* b : OnSelectBehaviours)
	{
		b->OnHoverEndEvent.Broadcast(IntenSelect);
	}
}

void UIntenSelectable::HandleOnClickStartEvents(UIntenSelectComponent* IntenSelect)
{
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		FInputActionValue v;
		b->OnClickStartEvent.Broadcast(IntenSelect, v);
	}
}

void UIntenSelectable::HandleOnClickEndEvents(UIntenSelectComponent* IntenSelect, FInputActionValue& InputValue)
{
	for(const UClickBehaviour* b : OnClickBehaviours)
	{
		b->OnClickEndEvent.Broadcast(IntenSelect, InputValue);
	}
}

void UIntenSelectable::InitDefaultBehaviourReferences()
{
	//Scoring
	if(UIntenSelectableScoring* AttachedScoring = Cast<UIntenSelectableScoring>(GetOwner()->GetComponentByClass(UIntenSelectableScoring::StaticClass())))
	{
		ScoringBehaviour = AttachedScoring;
	}else
	{
		ScoringBehaviour = NewObject<UIntenSelectableSinglePointScoring>(this, UIntenSelectableSinglePointScoring::StaticClass(), "Default Scoring");
		ScoringBehaviour->SetWorldLocation(GetOwner()->GetActorLocation());
		ScoringBehaviour->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	//Selecting
	TInlineComponentArray<UHoverBehaviour*> AttachedSelectionBehaviours;
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
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("RUNTIME ERROR")));
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
