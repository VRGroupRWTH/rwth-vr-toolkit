#include "DisplayClusterExtensions.h"

#define LOCTEXT_NAMESPACE "FnDisplayExtensionsModule"

void FDisplayClusterExtensionsModule::StartupModule ()
{
	StereoDeviceFix.Register();
}
void FDisplayClusterExtensionsModule::ShutdownModule()
{
	StereoDeviceFix.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDisplayClusterExtensionsModule, DisplayClusterExtensions)