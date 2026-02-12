// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Stealth.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitVelocityChange.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWGA_Stealth::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 3.0f);
	WaitDelayTask->OnFinish.AddDynamic(this, &UPTWGA_Stealth::OnStealthTimerFinished);
	WaitDelayTask->ReadyForActivation();
}


void UPTWGA_Stealth::OnStealthTimerFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		if (IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo()))
		{
			CombatInterface->ApplyGameplayEffectToSelf(StealthEffectClass, 1.0f, FGameplayEffectContextHandle());
		}
	}
	
	UAbilityTask_WaitGameplayTagRemoved* TagRemovedTask = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(this, FGameplayTag::RequestGameplayTag(FName("State.Idle")));
	TagRemovedTask->Removed.AddDynamic(this, &UPTWGA_Stealth::OnMovementDetected);
	TagRemovedTask->ReadyForActivation();
}

void UPTWGA_Stealth::OnMovementDetected()
{
	if (IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo()))
	{
		CombatInterface->RemoveEffectWithTag(GameplayTags::State::Stealth);
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

