// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Blind.generated.h"

/**
 * 
 */

UCLASS()
class PTW_API UPTWGA_Blind : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Effects")
	TSubclassOf<UGameplayEffect> BlindEffect;
};
