#include "RWTHVRToolkit.h"

#define LOCTEXT_NAMESPACE "FRWTHVRToolkitModule"

void FRWTHVRToolkitModule::StartupModule()
{
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient* LiveLinkClient = static_cast<FLiveLinkClient*>(&IModularFeatures::Get().GetModularFeature<
			ILiveLinkClient>(
			ILiveLinkClient::ModularFeatureName));
		LiveLinkMotionController = MakeUnique<FLiveLinkMotionControllerFix>(*LiveLinkClient);
		LiveLinkMotionController->RegisterController();
	}

	ConsoleActivation.Register();
}

void FRWTHVRToolkitModule::ShutdownModule()
{
	ConsoleActivation.Unregister();
	if (LiveLinkMotionController)
		LiveLinkMotionController->UnregisterController();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRWTHVRToolkitModule, RWTHVRToolkit)
