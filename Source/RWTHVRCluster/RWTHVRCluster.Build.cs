using UnrealBuildTool;

public class RWTHVRCluster : ModuleRules
{
	public RWTHVRCluster(ReadOnlyTargetRules Target) : base(Target)
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
			"RWTHVRToolkit",
			"DisplayCluster",
			"DeveloperSettings",
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]{}
		);

        DynamicallyLoadedModuleNames.AddRange(
            new string[] { }
        );
    }
}
