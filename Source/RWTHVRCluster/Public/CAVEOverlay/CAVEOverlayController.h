#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Pawn/VirtualRealityPawn.h"
#include "CAVEOverlayController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCAVEOverlay, Log, All);

UCLASS()
class RWTHVRCLUSTER_API ACAVEOverlayController : public AActor
{
	GENERATED_BODY()

public:
	ACAVEOverlayController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	//Execution Modes
	bool bCAVEMode = false;

	//Screen Types
	enum EScreen_Type { SCREEN_MASTER, SCREEN_NORMAL, SCREEN_DOOR_PARTIAL, SCREEN_DOOR };

	EScreen_Type ScreenType = SCREEN_NORMAL;
	const TArray<FString> ScreensDoor = {
		"node_bul_left_eye", "node_bul_right_eye", "node_bll_left_eye", "node_bll_right_eye"
	};
	const TArray<FString> ScreensDoorPartial = {
		"node_bur_left_eye", "node_bur_right_eye", "node_blr_left_eye", "node_blr_right_eye"
	};

	const FString ScreenMain = "node_main";

	//Door Mode
	enum EDoorMode { DOOR_PARTIALLY_OPEN = 0, DOOR_OPEN = 1, DOOR_CLOSED = 2, DOOR_DEBUG = 3, DOOR_NUM_MODES = 4 };

	const FString DoorModeNames[DOOR_NUM_MODES] = {"Partially Open", "Open", "Closed", "Debug"};
	EDoorMode DoorCurrentMode = DOOR_PARTIALLY_OPEN;
	const float DoorOpeningWidthRelative = 0.522; //%, used for the overlay width on the screen
	const float DoorOpeningWidthAbsolute = 165; //cm, used for the non tape part at the door
	const float WallDistance = 262.5; //cm, distance from center to a wall, *2 = wall width
	const float WallCloseDistance = 75; //cm, the distance considered to be too close to the walls
	const float WallFadeDistance = 35; //cm, the distance over which the tape is faded
	const float WallWarningDistance = 40; //cm, distance on which the tape turns red, measured from wall
	float DoorCurrentOpeningWidthAbsolute = 0;

	//Geometry and Material
	UStaticMeshComponent* CreateMeshComponent(const FName& Name, USceneComponent* Parent);

	double CalculateOpacityFromPosition(const FVector& Position) const;
	bool PositionInDoorOpening(const FVector& Position) const;
	void SetSignsForHand(UStaticMeshComponent* Sign, const FVector HandPosition, UMaterialInstanceDynamic* HandMaterial) const;

	bool bInitialized = false;

	UPROPERTY()
	AVirtualRealityPawn* VRPawn;

	//Cluster Events
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
	void HandleClusterEvent(const FDisplayClusterClusterEventJson& Event);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CycleDoorType();
	void SetDoorMode(EDoorMode M);

	//Signs and Banners
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Tape;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignRightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignLeftHand;

	//Overlay

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "CAVEOverlay")
	TSubclassOf<UDoorOverlayData> OverlayClass;

	UPROPERTY()
	UDoorOverlayData* Overlay;

	UPROPERTY()
	UMaterialInstanceDynamic* TapeMaterialDynamic;

	UPROPERTY()
	UMaterialInstanceDynamic* RightSignMaterialDynamic;

	UPROPERTY()
	UMaterialInstanceDynamic* LeftSignMaterialDynamic;
};
