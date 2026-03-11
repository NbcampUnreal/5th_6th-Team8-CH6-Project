// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTWAbilityDefinition.generated.h"


class UGameplayAbility;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EPTWAbilityTier : uint8
{
	Tier1,
	Tier2,
	Tier3
};

UCLASS()
class PTW_API UPTWAbilityDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FText Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EPTWAbilityTier Tier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;
};
