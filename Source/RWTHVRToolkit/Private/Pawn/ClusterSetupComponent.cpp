// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ClusterSetupComponent.h"

#include "IDisplayCluster.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Core/RWTHVRPlayerState.h"
#include "Utility/RWTHVRUtilities.h"
#include "Pawn/ClusterRepresentationActor.h"
#include "Pawn/VRClusterSyncComponent.h"

#if PLATFORM_SUPPORTS_CLUSTER
#include "Components/DisplayClusterSceneComponentSyncParent.h"
#endif

// Sets default values for this component's properties
UClusterSetupComponent::UClusterSetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
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

	// Server path — existing cluster actor attachment logic.
	if (Pawn->HasAuthority() && NewController)
	{
		if (const ARWTHVRPlayerState* State = Pawn->GetPlayerState<ARWTHVRPlayerState>())
		{
			const EPlayerType Type = State->GetPlayerType();
			if (Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary)
			{
				AttachClusterToPawn();
			}
		}
	}
 
	// Client path — UE 5.7: HasAuthority() is always false here on clients,
	// so the server-only branch above never fires the sync component add on
	// the client side. We call it directly instead. See OnRep_PlayerState in
	// AVRPawn for the event-driven retry that fires once PlayerState/PlayerType
	// has actually replicated, removing the old timer-based race condition.
	if (!Pawn->HasAuthority() && NewController && NewController->IsLocalController())
	{
		if (const ARWTHVRPlayerState* State = Pawn->GetPlayerState<ARWTHVRPlayerState>())
		{
			const EPlayerType Type = State->GetPlayerType();
			if (Type == EPlayerType::nDisplayPrimary || Type == EPlayerType::nDisplaySecondary)
			{
				MulticastAddDCSyncComponent();
			}
		}
		// If PlayerState/PlayerType isn't ready yet, AVRPawn::OnRep_PlayerState
		// will call MulticastAddDCSyncComponent() once it replicates in.
	}
}

void UClusterSetupComponent::MulticastAddDCSyncComponent_Implementation()
{
#if PLATFORM_SUPPORTS_CLUSTER
	if (URWTHVRUtilities::IsRoomMountedMode() && !SyncComponent)
	{
		// UVRClusterSyncComponent overrides GetSyncTransform/SetSyncTransform to sync the pawn
		// across cluster nodes (see that class for the primary/secondary split).
		SyncComponent = Cast<USceneComponent>(GetOwner()->AddComponentByClass(
			UVRClusterSyncComponent::StaticClass(), false, FTransform::Identity, false));
		GetOwner()->AddInstanceComponent(SyncComponent);
		UE_LOGFMT(Toolkit, Display, "UClusterSetupComponent: Added VRClusterSyncComponent to pawn {Pawn}", GetName());
	}
#endif
}
