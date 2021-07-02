#pragma once

#include "Components/DisplayClusterCameraComponent.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VirtualRealityUtilities.generated.h"

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

	/* TDW Specific */
	NCC_TDW_ORIGIN UMETA(DisplayName = "TDW Origin"),
	NCC_TDW_CENTER UMETA(DisplayName = "TDW Center"),

	/* Non Specific */
	NCC_SHUTTERGLASSES UMETA(DisplayName = "CAVE/ROLV/TDW Shutter Glasses"),
	NCC_FLYSTICK UMETA(DisplayName = "CAVE/ROLV/TDW Flystick"),
	NCC_TRACKING_ORIGIN UMETA(DisplayName = "CAVE/ROLV/TDW Origin")
};

UCLASS()
class RWTHVRTOOLKIT_API UVirtualRealityUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsDesktopMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsRoomMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsHeadMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsCave();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsTdw();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster|Platform") static bool IsRolv();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsMaster();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsSlave();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static FString GetNodeName();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static float GetEyeDistance();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static EDisplayClusterEyeStereoOffset GetNodeEyeType();

	//Get Compenent of Display Cluster by it's name, which is specified in the nDisplay config
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static UDisplayClusterSceneComponent* GetClusterComponent(const FString& Name);
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static UDisplayClusterSceneComponent* GetNamedClusterComponent(const ENamedClusterComponent& Component);
};
