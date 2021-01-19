#pragma once

#include "Components/DisplayClusterCameraComponent.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VirtualRealityUtilities.generated.h"

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API UVirtualRealityUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsDesktopMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsRoomMountedMode();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsHeadMountedMode();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsMaster();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static bool IsSlave();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static FString GetNodeName();
	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static float GetEyeDistance();

	UFUNCTION(BlueprintPure, Category = "DisplayCluster") static EDisplayClusterEyeStereoOffset GetNodeEyeType();

	//Get Compenent of Display Cluster by it's name, which is specified in the nDisplay config
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "DisplayCluster") static UDisplayClusterSceneComponent* GetClusterComponent(const FString& Name);
};
