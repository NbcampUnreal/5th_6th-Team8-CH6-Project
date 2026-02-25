// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Melee.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Melee : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
	void OnMeleeHitReceived(FGameplayEventData Payload);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeMontage")
	TObjectPtr<UAnimMontage> MeleeAttackMontage;
};
