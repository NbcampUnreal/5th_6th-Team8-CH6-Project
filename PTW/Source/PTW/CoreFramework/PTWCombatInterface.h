// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PTWCombatInterface.generated.h"

struct FGameplayEffectContextHandle;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPTWCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PTW_API IPTWCombatInterface
{
	GENERATED_BODY()
public:
	virtual float GetDamageMultiplier(const FName& BoneName) const = 0; // 데미지 배율 계산
	
	virtual void RemoveEffectWithTag(const FGameplayTag& TagToRemove) = 0;
	
	virtual void ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> EffectClass, float Level, FGameplayEffectContextHandle Context) = 0;
	
	virtual void ApplyGameplayEffectWithDuration(TSubclassOf<class UGameplayEffect> EffectClass, 
		float Level, 
		float Duration, 
		FGameplayEffectContextHandle Context) = 0;
	
};
