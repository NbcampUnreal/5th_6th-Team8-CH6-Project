// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Melee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event::Melee::Hit);
	
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMeleeHitReceived);
	WaitEventTask->ReadyForActivation();
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MeleeAttackMontage);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageTask->ReadyForActivation();
}

void UPTWGA_Melee::OnMeleeHitReceived(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit"));
	}
}
