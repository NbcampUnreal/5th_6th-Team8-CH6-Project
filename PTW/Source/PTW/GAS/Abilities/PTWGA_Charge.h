// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Charge.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Charge : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
protected:
	void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	void ApplyChargeEffect();
	void TickReCharge();
	void FinishRecharge();
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> ReChargeGEClass;
	
	FTimerHandle RechargeTimerHandle;
	
	UPROPERTY(EditAnywhere)
	float RechargeSpeed;
};
