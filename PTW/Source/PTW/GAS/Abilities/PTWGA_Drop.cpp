// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Drop.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"

void UPTWGA_Drop::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	UPTWInventoryComponent* InventoryComponent = PC->GetInventoryComponent();
	if (!InventoryComponent) return;
	
	InventoryComponent->DropItem();
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
