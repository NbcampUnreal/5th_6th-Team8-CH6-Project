// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "PTWItemSpawnManager.generated.h"

class UPTWItemDefinition;
class APTWWeaponActor;
class APTWPlayerCharacter;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemSpawnManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SpawnWeaponActor(APTWPlayerCharacter* TargetPlayer, UPTWItemDefinition* ItemDefinition, FGameplayTag WeaponTag);
	
};
