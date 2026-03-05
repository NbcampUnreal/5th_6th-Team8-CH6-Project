// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Blind.h"


void UPTWGA_Blind::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BlindEffect, GetAbilityLevel());
	
	if (SpecHandle.IsValid())
	{
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
