// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_InvisibilityCloak.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"

void UPTWGA_InvisibilityCloak::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	SettingVariable();
	
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
	
	ConsumeItem();
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_InvisibilityCloak::SettingVariable()
{
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!TargetCharacter) return;
	
	UPTWInventoryComponent* TargetInvenComp = TargetCharacter->GetInventoryComponent();
	if (!TargetInvenComp) return;
	
	PlayerCharacter = TargetCharacter;
	InventoryComponent = TargetInvenComp;
}
