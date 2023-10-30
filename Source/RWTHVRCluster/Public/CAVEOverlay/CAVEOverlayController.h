#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"
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
	virtual void PostInitializeComponents() override;

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
	const TArray<FString> ScreensFPS = {
		"node_rur_left_eye", "node_rur_right_eye", "node_lur_left_eye", "node_lur_right_eye", "node_main"
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

	//Overlay
	TSubclassOf<class UDoorOverlayData> OverlayClass;
	UPROPERTY()
	UDoorOverlayData* Overlay;

	//Geometry and Material
	UStaticMeshComponent* CreateMeshComponent(const FName& Name, UStaticMesh* Mesh, USceneComponent* Parent);
	UPROPERTY()
	UMaterial* TapeMaterial = nullptr;
	UPROPERTY()
	UMaterial* SignMaterial = nullptr;
	float CalculateOpacityFromPosition(const FVector& Position) const;
	bool PositionInDoorOpening(const FVector& Position) const;

	//Pawn Components
	bool bAttachedToCAVEOrigin = false;
	UPROPERTY()
	USceneComponent* CaveOrigin;
	UPROPERTY()
	USceneComponent* Head;
	UPROPERTY()
	USceneComponent* Flystick;

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
	USceneComponent* Root = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	USceneComponent* TapeRoot = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SignRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TapeNegativeY = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TapeNegativeX = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TapePositiveY = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TapePositiveX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignNegativeY = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignNegativeX = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignPositiveY = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SignPositiveX = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* TapeMaterialDynamic = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* SignMaterialDynamic = nullptr;
	UPROPERTY()
	UStaticMesh* PlaneMesh = nullptr;
};
