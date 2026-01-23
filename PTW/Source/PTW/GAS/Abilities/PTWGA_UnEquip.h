// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_UnEquip.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_UnEquip : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_UnEquip();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
};
