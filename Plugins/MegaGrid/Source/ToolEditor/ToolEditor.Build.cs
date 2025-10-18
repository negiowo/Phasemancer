// Copyright Two Bit Studios. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ToolEditor : ModuleRules
{
	public ToolEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "EditorMode"),
                Path.Combine(ModuleDirectory, "../GridPlugin/Data"),
                Path.Combine(ModuleDirectory, "../GridPlugin/Utils"),
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "EditorMode"),
                Path.Combine(ModuleDirectory, "../GridPlugin/Data"),
                Path.Combine(ModuleDirectory, "../GridPlugin/Utils"),
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "Engine",
                "CoreUObject",
                "InputCore",
                "LevelEditor",
                "Slate",
                "EditorStyle",
                "AssetTools",
                "EditorWidgets",
                "UnrealEd",
                "BlueprintGraph",
                "AnimGraph",
                "ComponentVisualizers",
                "EditorFramework",
                "GridPlugin",
                "Projects"
        }
        );


        PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                "Core",
                "CoreUObject",
                "Engine",
                "AppFramework",
                "SlateCore",
                "AnimGraph",
                "UnrealEd",
                "KismetWidgets",
                "MainFrame",
                "PropertyEditor",
                "ComponentVisualizers",
                "EditorFramework",
                "GridPlugin"
                    }
                    );


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");  // Ensures editor dependencies are included
        }
    }
}
