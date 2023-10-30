#include "RWTHVRCluster.h"

#define LOCTEXT_NAMESPACE "FRWTHVRClusterModule"

void FRWTHVRClusterModule::StartupModule ()
{
	ClusterConsole.Register();
	CAVEOverlay.Register();
}

void FRWTHVRClusterModule::ShutdownModule()
{
	ClusterConsole.Unregister();
	CAVEOverlay.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRClusterModule, RWTHVRCluster)
