// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TypicalShooter : ModuleRules
{
	public TypicalShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate", 
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks",
			"NetCore",
			"HarhalHexagonalGrid"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TypicalShooter",
			"TypicalShooter/Variant_Horror",
			"TypicalShooter/Variant_Horror/UI",
			"TypicalShooter/Variant_Shooter",
			"TypicalShooter/Variant_Shooter/AI",
			"TypicalShooter/Variant_Shooter/UI",
			"TypicalShooter/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
