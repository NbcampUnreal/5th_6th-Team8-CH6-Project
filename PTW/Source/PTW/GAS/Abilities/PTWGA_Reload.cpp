// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Reload.h"

#include "CoreFramework/PTWBaseCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"

UPTWGA_Reload::UPTWGA_Reload()
{
	//AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Reload")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Reload")));
}

void UPTWGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	//FIXME : 해당 코드 전부 수정 필요 (임시 테스트 코드)
	
	APTWBaseCharacter* AvatarActor = Cast<APTWBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (!AvatarActor) return;
	
	UAbilitySystemComponent* ASC = AvatarActor->GetAbilitySystemComponent();
	if (!ASC) return;
	
	UPTWInventoryComponent* InvenComp = AvatarActor->FindComponentByClass<UPTWInventoryComponent>();
	if (!InvenComp) return;
	
	int32 MaxAmmo = 0;
	
 	if (UPTWItemInstance* ItemInst = InvenComp->CurrentWeapon)       
	{
		if (APTWWeaponActor* WeaponActor = ItemInst->SpawnedWeapon1P)
		{
			if (UPTWWeaponData* Data =WeaponActor->GetWeaponData())
			{
				MaxAmmo = Data->MaxAmmo;
				InvenComp->CurrentWeapon->CurrentAmmo = MaxAmmo;
			}
		}
	}

	//TODO: UAbilityTask_PlayMontageAndWait 실행
	
	UE_LOG(LogTemp, Warning, TEXT("Reload Ability Activate CurrentWeapon Ammo : %d"), MaxAmmo);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
