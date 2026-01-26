// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Equip.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWBaseCharacter.h"
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
		APTWBaseCharacter* Character = GetPTWCharacterFromActorInfo();
		
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
