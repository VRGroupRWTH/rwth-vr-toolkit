#include "DisplayClusterExtensions.h"

#define LOCTEXT_NAMESPACE "FnDisplayExtensionsModule"

void FDisplayClusterExtensionsModule::StartupModule ()
{
	ClusterConsole.Register();
}
void FDisplayClusterExtensionsModule::ShutdownModule()
{
	ClusterConsole.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDisplayClusterExtensionsModule, DisplayClusterExtensions)