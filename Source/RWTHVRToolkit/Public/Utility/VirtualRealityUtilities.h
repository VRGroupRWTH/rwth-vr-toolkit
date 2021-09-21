#pragma once

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
	/* Distance in meters */
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static float GetEyeDistance();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static EEyeStereoOffset GetNodeEyeType();

	//Get Component of Display Cluster by it's name, which is specified in the nDisplay config
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static USceneComponent* GetClusterComponent(const FString& Name);
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static USceneComponent* GetNamedClusterComponent(const ENamedClusterComponent& Component);

	/* Load and create an Object from an asset path. This only works in the constructor */
    template <class T>
    static bool LoadAsset(const FString& Path, T*& Result);

    /* Finds and returns a class of an asset. This only works in the constructor */
    template <class T>
    static bool LoadClass(const FString& Path, TSubclassOf<T>& Result);
};

template <typename T>
bool UVirtualRealityUtilities::LoadAsset(const FString& Path, T* & Result)
{
	ConstructorHelpers::FObjectFinder<T> Loader(*Path);
	Result = Loader.Object;
	if (!Loader.Succeeded()) UE_LOG(LogTemp, Error, TEXT("Could not find %s. Have you renamed it?"), *Path);
	return Loader.Succeeded();
}

template <typename T>
bool UVirtualRealityUtilities::LoadClass(const FString& Path, TSubclassOf<T> & Result)
{
    ConstructorHelpers::FClassFinder<T> Loader(*Path);
	Result = Loader.Class;
	if (!Loader.Succeeded()) UE_LOG(LogTemp, Error, TEXT("Could not find %s. Have you renamed it?"), *Path);
	return Loader.Succeeded();
}
