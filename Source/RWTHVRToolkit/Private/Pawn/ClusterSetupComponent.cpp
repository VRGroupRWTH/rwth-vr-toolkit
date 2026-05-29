// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ClusterSetupComponent.h"

#include "Core/RWTHVRPlayerState.h"
#include "Utility/RWTHVRUtilities.h"
#include "Pawn/ClusterRepresentationActor.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "Components/DisplayClusterSceneComponentSyncParent.h"
#endif

// Sets default values for this component's properties
UClusterSetupComponent::UClusterSetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	// ...
}
void UClusterSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Cast<APawn>(GetOwner())
		->ReceiveControllerChangedDelegate.AddDynamic(this, &UClusterSetupComponent::OnNotifyControllerChanged);
}

// Executed on the server only: Attaches the ClusterRepresentation Actor, which contains the DCRA to the Pawn.
// It is only executed on the server because attachments are synced to all clients, but not from client to server.
void UClusterSetupComponent::AttachClusterToPawn()
{
	auto Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		UE_LOGFMT(Toolkit, Error, "UClusterSetupComponent::AttachClusterToPawn: Owner is not a Pawn!");
		return;
	}
	if (const ARWTHVRPlayerState* State = Pawn->GetPlayerState<ARWTHVRPlayerState>())
	{
		const auto ClusterActor = State->GetCorrespondingClusterActor();
		if (!ClusterActor)
		{
			UE_LOGFMT(Toolkit, Error,
					  "UClusterSetupComponent::AttachClusterToPawn: GetCorrespondingClusterActor returned null! This "
					  "won't work on "
					  "the Cave.");
			return;
		}
		const FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		bool bAttached = ClusterActor->AttachToComponent(GetOwner()->GetRootComponent(), AttachmentRules);
		// State->GetCorrespondingClusterActor()->OnAttached();
		UE_LOGFMT(Toolkit, Display,
				  "UClusterSetupComponent: Attaching corresponding cluster actor to our pawn returned: {Attached}",
				  bAttached);
	}
	else
	{
		UE_LOGFMT(
			Toolkit, Error,
			"UClusterSetupComponent::AttachClusterToPawn: No ARWTHVRPlayerState set! This won't work on the Cave.");
	}

	if (GetOwner()->HasAuthority()) // Should always be the case here, but double check
		MulticastAddDCSyncComponent();
}

/*
 * The alternative would be to do this only on the server on possess and check for player state/type,
 * as connections now send their playertype over.
 */
// This pawn's controller has changed! This is called on both server and owning client. If we are the owning client
// and the master, request that the Cluster is attached to us.
void UClusterSetupComponent::OnNotifyControllerChanged(APawn* Pawn, AController* OldController,
													   AController* NewController)
{
	// Try and use PlayerType for this:
	UE_LOG(Toolkit, Display, TEXT("UClusterSetupComponent: Player Controller has changed,"));

	if (Pawn->HasAuthority() && NewController)
	{
		UE_LOG(Toolkit, Display,
			   TEXT("UClusterSetupComponent: Player Controller has changed, trying to change Cluster attachment if "
					"possible..."));
		if (const ARWTHVRPlayerState* State = Pawn->GetPlayerState<ARWTHVRPlayerState>())
		{
			const EPlayerType Type = State->GetPlayerType();

			// Only cluster types are valid here as they are set on connection.
			// For all other player types this is a race condition
			if (Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary)
			{
				UE_LOGFMT(Toolkit, Display, "UClusterSetupComponent: Attaching Cluster to Pawn {Pawn}.", GetName());
				AttachClusterToPawn();
			}
		}
		else
		{
			UE_LOGFMT(Toolkit, Warning,
					  "UClusterSetupComponent: PlayerState is not a subclass of ARWTHVRPlayerState. Cluster attachment "
					  "only works "
					  "with correct PlayerStates!");
		}
	}
}

void UClusterSetupComponent::MulticastAddDCSyncComponent_Implementation()
{
#if PLATFORM_SUPPORTS_CLUSTER
	// Add an nDisplay Parent Sync Component. It syncs the parent's transform from master to clients.
	// This is required because for collision based movement, it can happen that the physics engine
	// for some reason acts different on the nodes, therefore leading to a potential desync when
	// e.g. colliding with an object while moving.

	if (URWTHVRUtilities::IsRoomMountedMode() && !SyncComponent)
	{
		SyncComponent = Cast<USceneComponent>(GetOwner()->AddComponentByClass(
			UDisplayClusterSceneComponentSyncParent::StaticClass(), false, FTransform::Identity, false));
		GetOwner()->AddInstanceComponent(SyncComponent);
		UE_LOGFMT(Toolkit, Display, "RWTHVRPawn: Added Sync Component to pawn {Pawn}", GetName());
	}
#endif
}
