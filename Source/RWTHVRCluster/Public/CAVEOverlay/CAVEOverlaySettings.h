#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CAVEOverlaySettings.generated.h"

UENUM(BlueprintType)
enum DefaultActivationType
{
	DefaultActivationType_OFF UMETA(DisplayName = "Off by default"),
	DefaultActivationType_ON UMETA(DisplayName = "On by default")
};

UCLASS(config=Game, defaultconfig, meta=(DisplayName="CAVE Overlay"))
class RWTHVRCLUSTER_API UCAVEOverlaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (DisplayName = "Default Activation Type"))
	TEnumAsByte<DefaultActivationType> DefaultActivationType = DefaultActivationType_ON;

	UPROPERTY(EditAnywhere, config, Category = Maps, meta=(AllowedClasses="World"))
	TArray<FSoftObjectPath> excludedMaps;
};
