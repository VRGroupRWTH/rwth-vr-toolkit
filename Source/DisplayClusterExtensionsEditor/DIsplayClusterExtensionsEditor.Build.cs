using UnrealBuildTool;

public class DisplayClusterExtensionsEditor : ModuleRules
{
    public DisplayClusterExtensionsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[] 
            { 
                "Core",
			    "CoreUObject",
			    "DisplayCluster",
				"Engine",
				"UnrealEd",
				"ComponentVisualizers",
				"HeadMountedDisplay",
				"InputCore",
				"DisplayClusterExtensions"


                // ... add public dependencies that you statically link with here ...   
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
              

                // ... add private dependencies that you statically link with here ...    
            }
        );

        PublicIncludePaths.AddRange(
          new string[] 
            {
         
                // ... add private dependencies that you statically link with here ...    
            }
        );

        PrivateIncludePaths.AddRange(
           new string[] 
            {
            
                // ... add private dependencies that you statically link with here ...    
            }
        );
    }
}