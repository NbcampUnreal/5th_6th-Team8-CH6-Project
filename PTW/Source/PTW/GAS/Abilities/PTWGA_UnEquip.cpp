// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_UnEquip.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/PTWItemInstance.h"

UPTWGA_UnEquip::UPTWGA_UnEquip()
{
	
}

void UPTWGA_UnEquip::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const UPTWItemInstance* WeaponItemInstance = Cast<UPTWItemInstance>(TriggerEventData->OptionalObject);
	
	if (WeaponItemInstance)
	{
		FGameplayTag CurrentWeaponTag = WeaponItemInstance->ItemDef->WeaponTag;
		APTWBaseCharacter* Character = GetPTWCharacterFromActorInfo();
		Character->EquipWeaponByTag(CurrentWeaponTag);
	
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
		if (!ASC) return;
	
		FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
		ASC->RemoveLooseGameplayTag(EquipTag);
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
