// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_BombPistol_Handler.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "CoreFramework/PTWCombatInterface.h"

void UPTWGA_BombPistol_Handler::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	
	if (!ASC) return;
	
	if (ASC->HasMatchingGameplayTag(BombTag))
	{
		StartWaitTagRemoved(BombTag);
	}
	else
	{
		UAbilityTask_WaitGameplayTagAdded* WaitAdded = UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(this, BombTag);
		WaitAdded->Added.AddDynamic(this, &UPTWGA_BombPistol_Handler::OnBombTagAdded);
		WaitAdded->ReadyForActivation();
	}
	
}

void UPTWGA_BombPistol_Handler::OnBombTagRemoved()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo());
	
	FGameplayTag TransferredTag = FGameplayTag::RequestGameplayTag(FName("Event.MiniGame.BombTransferred"));
	FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	
	if (ASC && CombatInt && ASC->HasMatchingGameplayTag(TransferredTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Explosion"));
		CombatInt->RemoveEffectWithTag(TransferredTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Explosion"));
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_BombPistol_Handler::OnBombTagAdded()
{
	FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	StartWaitTagRemoved(BombTag);
}

void UPTWGA_BombPistol_Handler::StartWaitTagRemoved(const FGameplayTag& BombTag)
{
	UAbilityTask_WaitGameplayTagRemoved* WaitTagRemovedTask = UAbilityTask_WaitGameplayTagRemoved::
		WaitGameplayTagRemove(
		this, 
		BombTag
	);
	
	if (WaitTagRemovedTask)
	{
		WaitTagRemovedTask->Removed.AddDynamic(this, &UPTWGA_BombPistol_Handler::OnBombTagRemoved);
		WaitTagRemovedTask->ReadyForActivation();
	}
}
