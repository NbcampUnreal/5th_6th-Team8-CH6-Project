// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_GimmickCollect.generated.h"

UCLASS()
class PTW_API UPTWGA_GimmickCollect : public UPTWGameplayAbility
{
	GENERATED_BODY()

public:
	UPTWGA_GimmickCollect();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

