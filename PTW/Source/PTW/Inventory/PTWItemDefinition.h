// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTWItemDefinition.generated.h"

class UPTWGameplayAbility;
class APTWWeaponActor;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef")
	TSubclassOf<APTWWeaponActor> WeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef")
	TSubclassOf<UPTWGameplayAbility> AbilityToGrant;	
};
