// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ColourWars : ModuleRules
{
	public ColourWars(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
