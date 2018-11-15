using UnrealBuildTool;

public class nDisplayExtensions : ModuleRules
{
  public nDisplayExtensions(ReadOnlyTargetRules Target) : base(Target)
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
