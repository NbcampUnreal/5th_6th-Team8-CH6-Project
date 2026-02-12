// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Ice.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"

void UPTWGA_Ice::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Ice::ApplyItemEffect()
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(PC);
	if (!CombatInterface) return;
	
	CombatInterface->ApplyGameplayEffectToSelf(EffectClass, 1.0f, FGameplayEffectContextHandle());
}
