using UnrealBuildTool;

public class RWTHVRToolkit : ModuleRules
{
  public RWTHVRToolkit(ReadOnlyTargetRules Target) : base(Target)
  {
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    PublicIncludePaths.AddRange(
      new string[]
      {

      }
      );


    PrivateIncludePaths.AddRange(
      new string[]
      {

      }
      );


    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        "CoreUObject",
        "DisplayCluster",
        "Engine",
        "HeadMountedDisplay",
        "InputCore",
        "UMG",
        "Slate",
        "SlateCore",
        "DeveloperSettings"
      }
      );


    PrivateDependencyModuleNames.AddRange(
      new string[]
      {

      }
      );


    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {

      }
      );
  }
}
