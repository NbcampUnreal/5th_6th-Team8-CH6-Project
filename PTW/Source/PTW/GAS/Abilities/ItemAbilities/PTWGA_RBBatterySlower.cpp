// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_RBBatterySlower.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWGA_RBBatterySlower::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo());
	if (!CombatInterface) return;
	
	CombatInterface->RemoveEffectWithTag(GameplayTags::MiniGame::Delivery);
	CombatInterface->ApplyGameplayEffectToSelf(BatterySlowerEffect, 1.0f, FGameplayEffectContextHandle());
}
