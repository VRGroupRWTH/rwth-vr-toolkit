#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Pawn/RWTHVRPawn.h"
#include "CAVEOverlayController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCAVEOverlay, Log, All);

/**
 * Actor which controls the cave overlay. The overlay displays a warning tape around the cave
 * when the user moves their head too close to the wall, and a warning sign when the hands are
 * too close.
 */
UCLASS()
class RWTHVRCLUSTER_API ACAVEOverlayController : public AActor
{
	GENERATED_BODY()

public:
	ACAVEOverlayController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Types of cave screens defined in the cluster config.
	enum EScreen_Type
	{
		// the primary node screen
		SCREEN_PRIMARY,
		// any secondary node screen
		SCREEN_NORMAL,
		// the screens that cover the partially opened door
		SCREEN_DOOR_PARTIAL,
		// additional screens that cover the door
		SCREEN_DOOR
	};

	// which screen type this node is running on
	EScreen_Type ScreenType = SCREEN_NORMAL;

	// which additional node names define the screens that cover the door
	const TArray<FString> ScreensDoor = {"node_bul_left_eye", "node_bul_right_eye", "node_bll_left_eye",
										 "node_bll_right_eye"};

	// which node names define the screens that cover the partial door
	const TArray<FString> ScreensDoorPartial = {"node_bur_left_eye", "node_bur_right_eye", "node_blr_left_eye",
												"node_blr_right_eye"};

	const FString ScreenMain = "node_main";

	// Door Mode
	enum EDoorMode
	{
		DOOR_PARTIALLY_OPEN = 0,
		DOOR_OPEN = 1,
		DOOR_CLOSED = 2,
		DOOR_DEBUG = 3,
		DOOR_NUM_MODES = 4
	};

	const FString DoorModeNames[DOOR_NUM_MODES] = {"Partially Open", "Open", "Closed", "Debug"};
	EDoorMode DoorCurrentMode = DOOR_PARTIALLY_OPEN;
	const float DoorOpeningWidthRelative = 0.522; //%, used for the overlay width on the screen
	const float DoorOpeningWidthAbsolute = 165; // cm, used for the non tape part at the door
	const float WallDistance = 262.5; // cm, distance from center to a wall, *2 = wall width
	const float WallCloseDistance = 75; // cm, the distance considered to be too close to the walls
	const float WallFadeDistance = 35; // cm, the distance over which the tape is faded
	const float WallWarningDistance = 40; // cm, distance on which the tape turns red, measured from wall
	float DoorCurrentOpeningWidthAbsolute = 0;

	// Helper function to create a mesh component in the constructor
	UStaticMeshComponent* CreateMeshComponent(const FName& Name, USceneComponent* Parent);

	// Calculates opacity value used for the dynamic materials of the tape and sign. The closer the more opaque.
	double CalculateOpacityFromPosition(const FVector& Position) const;

	// Check whether the given position is within the door area of the (partially) open door.
	bool PositionInDoorOpening(const FVector& Position) const;

	// Sets the position, orientation and opacity/visibility of the Sign according to the HandPosition.
	void SetSignsForHand(UStaticMeshComponent* Sign, const FVector& HandPosition,
						 UMaterialInstanceDynamic* HandMaterial) const;

	// Only calculate positions and material values when we're fully initialized.
	bool bInitialized = false;

	// Reference to the currently active pawn that we're tracking positions of.
	UPROPERTY()
	ARWTHVRPawn* VRPawn;

	// Cluster Events
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
	void HandleClusterEvent(const FDisplayClusterClusterEventJson& Event);

public:
	virtual void Tick(float DeltaTime) override;

	// Change door mode manually between open, partially open and closed.
	void CycleDoorType();
	void SetDoorMode(EDoorMode M);

	// Root component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	// Tape Static Mesh component. Reference to static mesh needs to be set in the corresponding BP.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Tape;

	// Right Hand Sign Static Mesh component. Reference to static mesh needs to be set in the corresponding BP.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignRightHand;

	// Left Hand Sign Static Mesh component. Reference to static mesh needs to be set in the corresponding BP.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignLeftHand;

	// UI Overlay
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "CAVEOverlay")
	TSubclassOf<UDoorOverlayData> OverlayClass;

	// UI Overlay
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "CAVEOverlay")
	UInputAction* CycleDoorTypeInputAction;

	UPROPERTY()
	UDoorOverlayData* Overlay;

	// Dynamic Materials to control opacity
	UPROPERTY()
	UMaterialInstanceDynamic* TapeMaterialDynamic;

	UPROPERTY()
	UMaterialInstanceDynamic* RightSignMaterialDynamic;

	UPROPERTY()
	UMaterialInstanceDynamic* LeftSignMaterialDynamic;
};
