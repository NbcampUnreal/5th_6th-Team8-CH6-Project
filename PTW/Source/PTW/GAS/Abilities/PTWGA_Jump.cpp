// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Jump.h"
#include "CoreFramework/PTWBaseCharacter.h"

UPTWGA_Jump::UPTWGA_Jump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Posture.Crouching")));
}

void UPTWGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (APTWBaseCharacter* Character = GetPTWCharacterFromActorInfo())
	{
		Character->Jump();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
