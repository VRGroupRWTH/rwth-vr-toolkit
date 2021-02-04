#include "DisplayClusterExtensions.h"

#define LOCTEXT_NAMESPACE "FnDisplayExtensionsModule"

void FDisplayClusterExtensionsModule::StartupModule ()
{
	StereoDeviceFix.Register();
	CAVEOverlay.Register();
}
void FDisplayClusterExtensionsModule::ShutdownModule()
{
	StereoDeviceFix.Unregister();
	CAVEOverlay.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDisplayClusterExtensionsModule, DisplayClusterExtensions)