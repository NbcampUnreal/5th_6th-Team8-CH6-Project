// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_Fire.h"
#include "PTWGA_StunGun.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_StunGun : public UPTWGA_Fire
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage) override;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> StunEffect;
};
