// Copyright Two Bit Studios. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GridPlugin : ModuleRules
{
	public GridPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
		    new string[] {
		        Path.Combine(ModuleDirectory, "Data"),
		        Path.Combine(ModuleDirectory, "Pathfinding"),
		        Path.Combine(ModuleDirectory, "Thread"),
		        Path.Combine(ModuleDirectory, "Utils"),
		        Path.Combine(ModuleDirectory, "Visuals")
		    }
		);

        PrivateIncludePaths.AddRange(
		           new string[] {
		       Path.Combine(ModuleDirectory, "Private"),
		       Path.Combine(ModuleDirectory, "Public")
		           }
		       );

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
				// ... add other public dependencies that you statically link with here ...
			}
			);


        PrivateDependencyModuleNames.AddRange(
		    new string[]
		    {
		        "CoreUObject",
		        "Engine",
		        "Slate",
		        "SlateCore"
		        // ... add private dependencies that you statically link with here ...
		    }
		);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
