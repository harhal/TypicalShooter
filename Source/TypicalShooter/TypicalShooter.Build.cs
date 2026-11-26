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
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate", 
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks",
			"NetCore",
			"DeveloperSettings",
			"SlateCore", 
			"HarhalHexagonalGrid"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore" // IPlatformInputDeviceMapper (automation tests)
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd"); // PIE latent commands in automation tests
			PrivateDependencyModuleNames.AddRange(new string[] { "StateTreeEditorModule", "PropertyBindingUtils" }); // BuildEnemyStateTree commandlet
		}

		PublicIncludePaths.AddRange(new string[] {
			"TypicalShooter",
			"TypicalShooter/Variant_Horror",
			"TypicalShooter/Variant_Horror/UI",
			"TypicalShooter/Variant_Shooter",
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
