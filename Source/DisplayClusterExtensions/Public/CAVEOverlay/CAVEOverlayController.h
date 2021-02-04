#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAVEOverlay/DoorOverlayData.h"
#include "VirtualRealityPawn.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "CAVEOverlayController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCAVEOverlay, Log, All);

UCLASS()
class DISPLAYCLUSTEREXTENSIONS_API ACAVEOverlayController : public AActor
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
	bool bCAVE_Mode = false;

	//Screen Types
	enum EScreen_Type { SCREEN_MASTER, SCREEN_NORMAL, SCREEN_DOOR_PARTIAL, SCREEN_DOOR };

	EScreen_Type Screen_Type = SCREEN_NORMAL;
	const TArray<FString> Screens_Door = {"node_bul_left_eye", "node_bul_right_eye", "node_bll_left_eye", "node_bll_right_eye"};
	const TArray<FString> Screens_Door_Partial = {"node_bur_left_eye", "node_bur_right_eye", "node_blr_left_eye", "node_blr_right_eye"};
	const TArray<FString> Screens_FPS = {"node_rur_left_eye", "node_rur_right_eye", "node_lur_left_eye", "node_lur_right_eye", "node_main"};
	const FString Screen_Main = "node_main";

	//Door Mode
	enum EDoor_Mode { DOOR_PARTIALLY_OPEN = 0, DOOR_OPEN = 1, DOOR_CLOSED = 2, DOOR_DEBUG = 3, DOOR_NUM_MODES = 4 };

	const FString Door_Mode_Names[DOOR_NUM_MODES] = {"Partially Open", "Open", "Closed", "Debug"};
	EDoor_Mode Door_Current_Mode = DOOR_PARTIALLY_OPEN;
	const float Door_Opening_Width_Relative = 0.522; //%, used for the overlay width on the screen
	const float Door_Opening_Width_Absolute = 165; //cm, used for the non tape part at the door
	const float Wall_Distance = 262.5; //cm, distance from center to a wall, *2 = wall width
	const float Wall_Close_Distance = 75; //cm, the distance considered to be too close to the walls
	const float Wall_Fade_Distance = 35; //cm, the distance over which the tape is faded
	const float Wall_Warning_Distance = 40; //cm, distance on which the tape turns red, measured from wall
	float Door_Current_Opening_Width_Absolute = 0;

	//Overlay
	TSubclassOf<class UDoorOverlayData> Overlay_Class;
	UPROPERTY() UDoorOverlayData* Overlay;

	//Geometry and Material
	UStaticMeshComponent* CreateMeshComponent(const FName& Name, UStaticMesh* Mesh, USceneComponent* Parent);
	UPROPERTY() UMaterial* Tape_Material = nullptr;
	UPROPERTY() UMaterial* Sign_Material = nullptr;
	float CalculateOpacityFromPosition(FVector Position);
	bool PositionInDoorOpening(FVector Position);

	//Pawn Components
	bool bAttached = false;
	void RefreshPawnComponents();
	UPROPERTY() AVirtualRealityPawn* Player_Pawn;
	UPROPERTY() USceneComponent* Cave_Origin;
	UPROPERTY() USceneComponent* Head;

	//Cluster Events
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
	void HandleClusterEvent(const FDisplayClusterClusterEventJson& Event);

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CycleDoorType();
	void SetDoorMode(EDoor_Mode M);

	//Signs and Banners
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) USceneComponent* Root = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) USceneComponent* Tape_Root = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) USceneComponent* Sign_Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Tape_Negative_Y = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Tape_Negative_X = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Tape_Positive_Y = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Tape_Positive_X = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Sign_Negative_Y = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Sign_Negative_X = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Sign_Positive_Y = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAVEOverlay", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent* Sign_Positive_X = nullptr;

	UPROPERTY() UMaterialInstanceDynamic* Tape_Material_Dynamic_ = nullptr;
	UPROPERTY() UMaterialInstanceDynamic* Sign_Material_Dynamic_ = nullptr;
	UPROPERTY() UStaticMesh* Plane_Mesh_ = nullptr;
};
