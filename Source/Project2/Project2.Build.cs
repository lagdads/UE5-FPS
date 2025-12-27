// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project2 : ModuleRules
{
	public Project2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"PhysicsCore",
            "RenderCore",
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Project2",
		});

		// 若使用 Slate UI，请取消注释下方依赖
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// 若需要在线功能，请取消注释下方依赖
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// 若使用 OnlineSubsystemSteam，需在 uproject 插件配置中启用该模块
	}
}
