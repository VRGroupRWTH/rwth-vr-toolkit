#pragma once

#include "CoreMinimal.h"
#include "DisplayClusterSceneComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VirtualRealityUtilities.generated.h"

UENUM(BlueprintType)
enum class EEyeType : uint8
{
	ET_MONO UMETA(DisplayName = "mono"),
	ET_STEREO_RIGHT UMETA(DisplayName = "stero_right"),
	ET_STEREO_LEFT UMETA(DisplayName = "stereo_left")
};

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API UVirtualRealityUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsDesktopMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsRoomMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsHeadMountedMode();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static FString GetNodeName();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static float GetEyeDistance();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static EEyeType GetNodeEyeType();

	//Get Compenent of Display Cluster by it's name, which is specified in the nDisplay config
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static UDisplayClusterSceneComponent* GetClusterComponent(const FString& Name);
};
