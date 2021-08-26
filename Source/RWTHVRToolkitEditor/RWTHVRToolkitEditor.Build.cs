using UnrealBuildTool;

public class RWTHVRToolkitEditor : ModuleRules
{
    public RWTHVRToolkitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[] 
            { 
                "Core",
			    "CoreUObject",
				"Engine",
				"UnrealEd",
				"ComponentVisualizers",
				"HeadMountedDisplay",
				"InputCore",
				"RWTHVRToolkit"


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