// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PTWItemDefinition.generated.h"

class UImage;

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
class UGameplayEffect;
class UTexture2D;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	TSubclassOf<UPTWGameplayAbility> AbilityToGrant;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	FGameplayTag ItemTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	TSoftObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDefault")
	FText DescriptionText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active",meta = (EditCondition = "ItemType == EItemType::Active", EditConditionHides))
	int32 MaxUsage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active", meta = (EditCondition = "ItemType == EItemType::Active", EditConditionHides))
	FGameplayTag CooldownTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	TSubclassOf<APTWWeaponActor> WeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	TSubclassOf<UGameplayEffect> EffectToGrant;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon",meta = (EditCondition = "ItemType == EItemType::Weapon", EditConditionHides))
	FGameplayTag WeaponTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive",meta = (EditCondition = "ItemType == EItemType::Passive", EditConditionHides))
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;
	

};
