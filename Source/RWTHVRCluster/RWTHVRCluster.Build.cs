using UnrealBuildTool;

public class RWTHVRCluster : ModuleRules
{
	public RWTHVRCluster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] { }
		);

		PrivateIncludePaths.AddRange(
			new string[] { }
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"InputCore",
				"UMG",
				"Slate",
				"SlateCore",
				"RWTHVRToolkit",
				"LiveLink"
			}
		);

		if (IsPluginEnabledForTarget("nDisplay", base.Target))
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"DisplayCluster"
				}
			);
		}

		if (IsPluginEnabledForTarget("DTrackPlugin", base.Target))
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"DTrackPlugin",
					"DTrackInput"
				}
			);
		}

		PrivateDependencyModuleNames.AddRange(
			new string[] { }
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] { }
		);
	}

	private static bool IsPluginEnabledForTarget(string PluginName, ReadOnlyTargetRules Target)
	{
		var PL = Plugins.GetPlugin(PluginName);
		return PL != null && Target.ProjectFile != null && Plugins.IsPluginEnabledForTarget(PL,
			ProjectDescriptor.FromFile(Target.ProjectFile), Target.Platform, Target.Configuration, Target.Type);
	}
}