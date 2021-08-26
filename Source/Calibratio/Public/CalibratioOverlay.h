#pragma once

#include "CoreMinimal.h"

#include "CalibratioActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "CalibratioOverlay.generated.h"


UENUM(BlueprintType)
enum ECalibratioProtocolStatus
{
	Calibrating,
	Waiting,
	Triggered
};

UENUM(BlueprintType)
enum ECalibratioPhysicalStatus
{
	Moving,
	Found,
	NotFound,
	Unknown
};

/**
 * This is the parent class for the Overlay that is used on the master.
 * All declarations in it are magically bound to the UMG child class if they are named the same (see "meta = (BindWidget)")
 */
UCLASS()
class CALIBRATIO_API UCalibratioOverlay : public UUserWidget
{
	GENERATED_BODY()

virtual bool Initialize() override;
public:
	/* Public Buttons */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* ResettingButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* IncreaseThresholdButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* DecreaseThresholdButton;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* DismissButton;

	/* Numbers: */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CurrentThreshold;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* MinAngle;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* MaxAngle;

	/* Status: */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatusProtocol;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatusCalibratio;

public:
	UFUNCTION() void SetStatus(ECalibratioProtocolStatus Status);
	UFUNCTION() void SetThresholds(float Min, float Max, float Threshold);
	UFUNCTION() void SetPhysicalStatus(ECalibratioPhysicalStatus Status);
	void SetOwner(ACalibratioActor* InOwner) {Owner = InOwner;}
	UFUNCTION() void Dismiss();

private:
	ECalibratioPhysicalStatus CurrentPhysicalStatus = Unknown;
	ECalibratioProtocolStatus CurrentStatus = Calibrating;
	UPROPERTY() ACalibratioActor* Owner;
};
