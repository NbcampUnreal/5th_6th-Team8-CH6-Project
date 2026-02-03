// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_ItemAbilityBase.h"
#include "PTWGA_Booster.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Booster : public UPTWGA_ItemAbilityBase
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyItemEffect() override;
};
