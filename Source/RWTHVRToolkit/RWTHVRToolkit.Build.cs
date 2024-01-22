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
			"HTTP",
			"LiveLink",
			"LiveLinkInterface",
			"EnhancedInput", 
			"Niagara",
			"NavigationSystem"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore"
			}
		);
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]{}
		);

		PublicDefinitions.Add(IsPluginEnabledForTarget("nDisplay", base.Target)
			? "PLATFORM_SUPPORTS_NDISPLAY=1"
			: "PLATFORM_SUPPORTS_NDISPLAY=0");
	}
	
	private static bool IsPluginEnabledForTarget(string PluginName, ReadOnlyTargetRules Target)
	{
		var PL = Plugins.GetPlugin(PluginName);
		return PL != null && Target.ProjectFile != null && Plugins.IsPluginEnabledForTarget(PL,
			ProjectDescriptor.FromFile(Target.ProjectFile), Target.Platform, Target.Configuration, Target.Type);
	}
}
