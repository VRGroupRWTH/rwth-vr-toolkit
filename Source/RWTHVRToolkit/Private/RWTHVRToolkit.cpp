#include "RWTHVRToolkit.h"

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule ()
{
	ConsoleActivation.Register();
}

void FRWTHVRToolkitModule::ShutdownModule()
{
	ConsoleActivation.Unregister();	
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRToolkitModule, RWTHVRToolkit)