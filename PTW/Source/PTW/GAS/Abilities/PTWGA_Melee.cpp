// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Melee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UPTWGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MeleeAttackMontage);
	MontageTask->ReadyForActivation();
}
