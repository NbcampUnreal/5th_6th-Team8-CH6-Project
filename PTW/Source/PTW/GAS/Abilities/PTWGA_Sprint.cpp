// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Sprint.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystemComponent.h"

UPTWGA_Sprint::UPTWGA_Sprint()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Sprinting")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Posture.Crouching")));
}

void UPTWGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && SprintEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SprintEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	UAbilityTask_WaitInputRelease* Task = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	Task->OnRelease.AddDynamic(this, &UPTWGA_Sprint::OnInputReleased);
	Task->ReadyForActivation();
}

void UPTWGA_Sprint::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && ActiveEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
