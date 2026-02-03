// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_ItemAbilityBase.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_InvisibilityCloak.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_InvisibilityCloak : public UPTWGA_ItemAbilityBase
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InitializeVariable() override;
	virtual void ApplyItemEffect() override;
};
