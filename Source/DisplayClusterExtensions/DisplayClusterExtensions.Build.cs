using UnrealBuildTool;

public class DisplayClusterExtensions : ModuleRules
{
  public DisplayClusterExtensions(ReadOnlyTargetRules Target) : base(Target)
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
        "DisplayClusterInput",
        "Engine",
        "InputCore"
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
