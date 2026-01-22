// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Reload.h"

#include "CoreFramework/PTWBaseCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"

void UPTWGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	APTWBaseCharacter* AvatarActor = Cast<APTWBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (!AvatarActor) return;
	
	UAbilitySystemComponent* ASC = AvatarActor->GetAbilitySystemComponent();
	if (!ASC) return;
	
	UPTWInventoryComponent* InvenComp = AvatarActor->FindComponentByClass<UPTWInventoryComponent>();
	if (!InvenComp) return;
	
	int32 MaxAmmo = InvenComp->GetCurrentWeaponActor()->SpawnedWeapon->GetWeaponData()->MaxAmmo;
	
	InvenComp->GetCurrentWeaponActor()->CurrentAmmo = MaxAmmo;
	
	UE_LOG(LogTemp, Warning, TEXT("Reload Ability Activate CurrentWeapon Ammo : %d"), MaxAmmo);
}
