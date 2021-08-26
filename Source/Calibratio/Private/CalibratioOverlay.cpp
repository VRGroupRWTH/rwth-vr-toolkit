#include "CalibratioOverlay.h"

bool UCalibratioOverlay::Initialize()
{
	const bool Result = Super::Initialize();
	DismissButton->OnClicked.AddDynamic(this, &UCalibratioOverlay::Dismiss);
	return Result;
}

void UCalibratioOverlay::SetStatus(const ECalibratioProtocolStatus Status)
{
	if(CurrentStatus == Status) return;
	switch(Status)
	{
	case Calibrating:
		StatusProtocol->SetText(FText::FromString(TEXT("Calibrating")));
		break;
	case Waiting:
		StatusProtocol->SetText(FText::FromString(TEXT("Waiting")));
		break;
	case Triggered:
		StatusProtocol->SetText(FText::FromString(TEXT("Triggered")));
		break;
	}
	CurrentStatus = Status;
}

void UCalibratioOverlay::SetThresholds(const float Min, const float Max, const float Threshold)
{
	MinAngle->SetText(FText::FromString(FString::Printf(TEXT("%.2f\u00B0"), FMath::Fmod(FMath::RadiansToDegrees(Min), 360))));
	MaxAngle->SetText(FText::FromString(FString::Printf(TEXT("%.2f\u00B0"), FMath::Fmod(FMath::RadiansToDegrees(Max), 360))));
	CurrentThreshold->SetText(FText::FromString(FString::Printf(TEXT("+-%.2f\u00B0s"), FMath::Fmod(FMath::RadiansToDegrees(Threshold), 360))));
}

void UCalibratioOverlay::SetPhysicalStatus(const ECalibratioPhysicalStatus Status)
{
	if(CurrentPhysicalStatus == Status) return;
	switch(Status)
	{
	case Found:
		StatusCalibratio->SetText(FText::FromString(TEXT("Found")));
		StatusCalibratio->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		break;
	case Unknown:
	case NotFound:
		StatusCalibratio->SetText(FText::FromString(TEXT("Not Found")));
		StatusCalibratio->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		break;
	case Moving:
		StatusCalibratio->SetText(FText::FromString(TEXT("Moving")));
		StatusCalibratio->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
		break;
	}
	CurrentPhysicalStatus = Status;
}

void UCalibratioOverlay::Dismiss()
{
	Owner->ClusterDespawn();
}
