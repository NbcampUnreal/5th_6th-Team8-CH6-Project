// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_ItemAbilityBase.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"

void UPTWGA_ItemAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_ItemAbilityBase::ConsumeItem()
{
	InventoryComponent->ConsumeActiveItem();
}
