using UnrealBuildTool;

public class RWTHVRToolkit : ModuleRules
{
	public RWTHVRToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[]{}
		);

		PrivateIncludePaths.AddRange(
			new string[]{}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			"Core",
			"CoreUObject",
			"Engine",
			"HeadMountedDisplay",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"DeveloperSettings",
			"Http"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]{}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]{}
		);
		
		if(Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicDependencyModuleNames.Add("DisplayCluster");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_NDISPLAY=1");
		}
		else
		{
			PublicDefinitions.Add("PLATFORM_SUPPORTS_NDISPLAY=0");
		}
	}
}
