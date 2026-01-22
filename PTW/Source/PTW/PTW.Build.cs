// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PTW : ModuleRules
{
	public PTW(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				ModuleDirectory
			}
		);

		// 1. Core & Basic Modules (기본 엔진 모듈)
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput"
		});

        // 2. GAS (Gameplay Ability System) 필수 모듈
        PublicDependencyModuleNames.AddRange(new string[] {
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks"
        });

        // 3. UMG & UI 모듈
        PublicDependencyModuleNames.AddRange(new string[] {
            "UMG"
        });

        // 4. Networking & Online Subsystem (스팀/세션 연동용)
        PublicDependencyModuleNames.AddRange(new string[] {
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });
        
        // 5. Niagara 이펙트 모듈
        PublicDependencyModuleNames.AddRange(new string[] {
	       "Niagara"
        });

        // Private Dependencies (내부적으로만 사용하는 모듈)
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
			"Json",
            "JsonUtilities",
			"NetCore"

		});
    }
}
