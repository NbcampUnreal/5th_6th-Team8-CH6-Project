// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_BombPistol_Handler.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_BombPistol_Handler : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UFUNCTION()
	void OnBombTagRemoved();
	
	UFUNCTION()
	void OnBombTagAdded();
	
	void StartWaitTagRemoved(const FGameplayTag& BombTag);
};
