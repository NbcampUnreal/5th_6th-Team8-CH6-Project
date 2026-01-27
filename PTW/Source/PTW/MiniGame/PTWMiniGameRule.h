// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWMiniGameRule.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct FPTWSpawnRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWLoadoutRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWCombatRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWWinConditionRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWTeamRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWDamageRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWRandomEventRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";
};

USTRUCT(BlueprintType)
struct FPTWMiniGameRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Spawn")
	FPTWSpawnRule SpawnRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Loadout")
	FPTWLoadoutRule LoadoutRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Combat")
	FPTWCombatRule CombatRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|WinCondition")
	FPTWWinConditionRule WinConditionRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Team")
	FPTWTeamRule TeamRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Damage")
	FPTWDamageRule DamageRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|RandomEvent")
	FPTWRandomEventRule RandomEventRule;

};
