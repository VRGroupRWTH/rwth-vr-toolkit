#include "DisplayClusterExtensions.h"

#define LOCTEXT_NAMESPACE "FnDisplayExtensionsModule"

void FDisplayClusterExtensionsModule::StartupModule ()
{
	ClusterConsole.Register();
	StereoDeviceFix.Register();
	CAVEOverlay.Register();
}
void FDisplayClusterExtensionsModule::ShutdownModule()
{
	ClusterConsole.Unregister();
	StereoDeviceFix.Unregister();
	CAVEOverlay.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDisplayClusterExtensionsModule, DisplayClusterExtensions)