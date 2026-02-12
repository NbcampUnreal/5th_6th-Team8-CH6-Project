// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Emote.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UPTWGA_Emote::UPTWGA_Emote()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Action.Emote")));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Status.Emoting")));

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPTWGA_Emote::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (EmoteMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			EmoteMontage,
			1.0f,
			NAME_None,
			false
		);

		Task->OnCompleted.AddDynamic(this, &UPTWGA_Emote::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UPTWGA_Emote::OnMontageInterrupted);
		Task->OnBlendOut.AddDynamic(this, &UPTWGA_Emote::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UPTWGA_Emote::OnMontageInterrupted);

		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UPTWGA_Emote::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_Emote::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
