// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_UnEquip.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWGA_UnEquip::UPTWGA_UnEquip()
{
	
}

void UPTWGA_UnEquip::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const UPTWWeaponInstance* WeaponItemInstance = Cast<UPTWWeaponInstance>(TriggerEventData->OptionalObject);
	
	if (WeaponItemInstance)
	{
		SaveCurrentAmmo(const_cast<UPTWWeaponInstance*>(WeaponItemInstance));
		
		FGameplayTag CurrentWeaponTag = WeaponItemInstance->ItemDef->WeaponTag;
		APTWPlayerCharacter* Character = GetPTWPlayerCharacterFromActorInfo();
		
		if (HasAuthority(&CurrentActivationInfo))
		{
			Character->GetWeaponComponent()->EquipWeaponByTag(CurrentWeaponTag);
		}
	
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
		if (!ASC) return;
	
		FGameplayTag EquipTag = GameplayTags::Weapon::State::Equip;
		ASC->RemoveLooseGameplayTag(EquipTag);
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UPTWGA_UnEquip::SaveCurrentAmmo(UPTWWeaponInstance* WeaponInstance)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;
	
	const UPTWWeaponAttributeSet* WeaponAttributes = Cast<UPTWWeaponAttributeSet>(ASC->GetAttributeSet(UPTWWeaponAttributeSet::StaticClass()));
	if (WeaponAttributes)
	{
		WeaponInstance->SetCurrentAmmo(WeaponAttributes->GetCurrentAmmo());
	}
}

