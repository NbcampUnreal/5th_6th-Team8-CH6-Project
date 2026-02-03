// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PTWItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon,
	Active,
	Passive
};


struct FGameplayTag;
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
	TSubclassOf<UPTWGameplayAbility> AbilityToGrant;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef", meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	TSubclassOf<APTWWeaponActor> WeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef",meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	FGameplayTag WeaponTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDef")
	EItemType ItemType;
};
