// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Crouch.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Crouch : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_Crouch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> CrouchEffectClass;

	FActiveGameplayEffectHandle ActiveEffectHandle;
};
