// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DedicatedServers : ModuleRules
{
	public DedicatedServers(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 1. Core & Basic Modules (기본 엔진 모듈)
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GameLiftServerSDK"
		});

        // 2. GAS (Gameplay Ability System) 필수 모듈
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
        });
    }
}
