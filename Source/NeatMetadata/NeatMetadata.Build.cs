// Copyright Viktor Pramberg. All Rights Reserved.

using UnrealBuildTool;

public class NeatMetadata : ModuleRules
{
	public NeatMetadata(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange
		(
			new string[]
			{
				"Core",
			}
		);
		
		PrivateDependencyModuleNames.AddRange
		(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayTags",
				"GameplayTagsEditor",
				"DeveloperSettings",
				"UnrealEd",
				"PropertyEditor",
				"ClassViewer",
				"InputCore",
				"BlueprintGraph",
			}
		);
	}
}
