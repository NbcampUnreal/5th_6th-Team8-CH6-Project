// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Equip.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"

UPTWGA_Equip::UPTWGA_Equip()
{
	//AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")));
}

void UPTWGA_Equip::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const UPTWItemInstance* WeaponItemInstance = Cast<UPTWItemInstance>(TriggerEventData->OptionalObject);
	if (WeaponItemInstance)
	{
		
		FGameplayTag CurrentWeaponTag = WeaponItemInstance->ItemDef->WeaponTag;
		APTWPlayerCharacter* Character = GetPTWPlayerCharacterFromActorInfo();
		
		SetCharacterWeaponAttribute(WeaponItemInstance, Character);
		
		if (HasAuthority(&CurrentActivationInfo))
		{
			Character->EquipWeaponByTag(CurrentWeaponTag);
			 if (UPTWInventoryComponent* InvenComp = Character->FindComponentByClass<UPTWInventoryComponent>())
			 {
			 	InvenComp->SetCurrentWeaponInst(WeaponItemInstance);
			 }
		}
		
		FGameplayTag StatTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
		
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		
		if (!ASC) return;
		ASC->AddLooseGameplayTag(StatTag);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Equip::SetCharacterWeaponAttribute(const UPTWItemInstance* WeaponItemInstance,
	APTWPlayerCharacter* Character)
{
	if (WeaponItemInstance)
	{
		if (HasAuthority(&CurrentActivationInfo))
		{
			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
			if (!ASC) return;
			
			APTWWeaponActor* SpawnedWeapon = WeaponItemInstance->SpawnedWeapon1P;
			if (!SpawnedWeapon) return;
			
			UPTWWeaponData* WeaponmData = SpawnedWeapon->GetWeaponData();
			if (!WeaponmData) return;
			
			ASC->SetNumericAttributeBase(UPTWWeaponAttributeSet::GetMaxAmmoAttribute(), WeaponmData->MaxAmmo);
			ASC->SetNumericAttributeBase(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute(), WeaponmData->MaxAmmo);
			ASC->SetNumericAttributeBase(UPTWWeaponAttributeSet::GetDamageAttribute(), WeaponmData->BaseDamage);
			
			//ASC->ForceReplication();
		}
	}
}
