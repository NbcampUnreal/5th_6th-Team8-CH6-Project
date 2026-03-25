// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_RBGiveCoin.h"

#include "CoreFramework/PTWPlayerCharacter.h"

void UPTWGA_RBGiveCoin::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PC->GetPlayerState());
	if (!PTWPS) return;
	
	PTWPS->AddGold(100);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
