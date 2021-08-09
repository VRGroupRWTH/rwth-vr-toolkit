#include "RWTHVRToolkit.h"

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule ()
{
	ClusterConsole.Register();
	CAVEOverlay.Register();
	ConsoleActivation.Register();
	Calibratio.Register();
}
void FRWTHVRToolkitModule::ShutdownModule()
{
	ClusterConsole.Unregister();
	CAVEOverlay.Unregister();
	ConsoleActivation.Unregister();
	Calibratio.Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRToolkitModule, RWTHVRToolkit)