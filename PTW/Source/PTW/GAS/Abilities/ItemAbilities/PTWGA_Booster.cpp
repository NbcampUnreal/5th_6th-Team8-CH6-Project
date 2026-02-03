// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Booster.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerCharacter.h"

void UPTWGA_Booster::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Booster::ApplyItemEffect()
{
	if (!PlayerCharacter) return;
	
	IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(PlayerCharacter);
	if (!CombatInt) return;
	
	UAbilitySystemComponent* ASC  = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	if (EffectClass)
	{
		CombatInt->ApplyGameplayEffectToSelf(EffectClass, 1.0f, Context);
	}
}
