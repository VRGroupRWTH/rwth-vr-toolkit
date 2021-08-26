using UnrealBuildTool;

public class Calibratio : ModuleRules
{
	public Calibratio(ReadOnlyTargetRules Target) : base(Target)
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
				"Core", "CoreUObject", "Engine", "UMG", "SlateCore", "DisplayCluster", "RWTHVRToolkit"
            }
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]{}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]{}
		);
	}
}
