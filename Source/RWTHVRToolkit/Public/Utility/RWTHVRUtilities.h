#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"

#include "RWTHVRUtilities.generated.h"


/**
 * Custom log category for all RWTHVRToolkit related components
 */
DECLARE_LOG_CATEGORY_EXTERN(Toolkit, Log, All);

UENUM(BlueprintType)
enum class ENamedClusterComponent : uint8
{
	/* CAVE Specific */
	NCC_CAVE_ORIGIN UMETA(DisplayName = "CAVE Origin"),
	NCC_CAVE_CENTER UMETA(DisplayName = "CAVE Center"),
	NCC_CAVE_LHT UMETA(DisplayName = "CAVE Left Hand Target"),
	NCC_CAVE_RHT UMETA(DisplayName = "CAVE Right Hand Target"),

	/* ROLV Specific */
	NCC_ROLV_ORIGIN UMETA(DisplayName = "ROLV Origin"),

	/* Non Specific */
	NCC_CALIBRATIO UMETA(DisplayName = "Calibratio Motion to Photon Measurement Device"),
	NCC_SHUTTERGLASSES UMETA(DisplayName = "CAVE/ROLV/TDW Shutter Glasses"),
	NCC_FLYSTICK UMETA(DisplayName = "CAVE/ROLV/TDW Flystick"),
	NCC_TRACKING_ORIGIN UMETA(DisplayName = "CAVE/ROLV/TDW Origin")
};

UENUM()
enum class EEyeStereoOffset
{
	None,
	Left,
	Right
};

UCLASS()
class RWTHVRTOOLKIT_API URWTHVRUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform")
	static bool IsDesktopMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform")
	static bool IsRoomMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform")
	static bool IsHeadMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform")
	static bool IsCave();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform")
	static bool IsRolv();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster")
	static bool IsPrimaryNode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster")
	static bool IsSecondaryNode();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster")
	static FString GetNodeName();
	/* Distance in meters */
	UFUNCTION(BlueprintPure, Category = "DisplayCluster")
	static float GetEyeDistance();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster")
	static EEyeStereoOffset GetNodeEyeType();

	//Get Component of Display Cluster by it's name, which is specified in the nDisplay config
	UE_DEPRECATED(5.4, "GetClusterComponent has been removed because it is obsolete.")
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster", meta = (DeprecatedFunction))
	static USceneComponent* GetClusterComponent(const FString& Name);

	UE_DEPRECATED(5.4, "GetNamedClusterComponent has been removed because it is obsolete.")
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster", meta = (DeprecatedFunction))
	static USceneComponent* GetNamedClusterComponent(const ENamedClusterComponent& Component);

	UFUNCTION(BlueprintCallable)
	static void ShowErrorAndQuit(UWorld* WorldContext, const FString& Message);
};
