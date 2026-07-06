#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "RWTHVRUtilities.generated.h"


/**
 * Custom log category for all RWTHVRToolkit related components
 */
DECLARE_LOG_CATEGORY_EXTERN(Toolkit, Log, All);

UCLASS()
class RWTHVRTOOLKIT_API URWTHVRUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RWTHVRToolkit|Platform")
	static bool IsDesktopMode();
	UFUNCTION(BlueprintPure, Category = "RWTHVRToolkit|Platform")
	static bool IsHeadMountedMode();
	UFUNCTION(BlueprintPure, Category = "RWTHVRToolkit|Platform")
	static bool IsRoomMountedMode();
	UFUNCTION(BlueprintPure, Category = "RWTHVRToolkit|Platform")
	static bool IsPrimaryNode();

	/* Distance in meters */
	UFUNCTION(BlueprintPure, Category = "RWTHVRToolkit")
	static float GetEyeDistance();

	UFUNCTION(BlueprintCallable)
	static void ShowErrorAndQuit(UWorld* WorldContext, const FString& Message);

	/**
	 * Returns true if the position or rotation delta between Current and Reference exceeds the deadzone.
	 * TO BE REMOVED: Might not be necessary
	 */
	static bool ExceedsTransformDeadzone(const FVector& CurrentLoc, const FRotator& CurrentRot,
										 const FVector& ReferenceLoc, const FRotator& ReferenceRot,
										 float PositionDeadzoneCm, float RotationDeadzoneDeg);
};
