#include "RWTHVRToolkit.h"

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule ()
{
	ClusterConsole.Register();
	StereoDeviceFix.Register();
	CAVEOverlay.Register();
	ConsoleActivation.Register();
}
void FRWTHVRToolkitModule::ShutdownModule()
{
	ClusterConsole.Unregister();
	StereoDeviceFix.Unregister();
	CAVEOverlay.Unregister();
	ConsoleActivation.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRToolkitModule, RWTHVRToolkit)