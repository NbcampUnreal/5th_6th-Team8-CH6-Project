// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Reload.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/PTWWeaponData.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWGA_Reload::UPTWGA_Reload()
{
	ActivationOwnedTags.AddTag(GameplayTags::Weapon::State::Reload);
}

void UPTWGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	UAnimMontage* MontageToPlay = GetReloadMontage(PC);

	if (!PC || !MontageToPlay || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.0f, NAME_None, false);
	
	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	
	PC->GetWeaponComponent()->PlayMontage1P(MontageToPlay);
	
	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTag EventTag = GameplayTags::Event::Weapon_ReloadReFill;
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGameplayEventReceived);
		WaitEventTask->ReadyForActivation();
	}
}

UAnimMontage* UPTWGA_Reload::GetReloadMontage(APTWPlayerCharacter* PC) const
{
	if (!PC) return nullptr;
    
	UPTWInventoryComponent* Inven = PC->GetInventoryComponent();
	if (!Inven) return nullptr;

	UPTWWeaponInstance* CurrentItem = Cast<UPTWWeaponInstance>(Inven->GetCurrentWeaponInst());
	if (!CurrentItem) return nullptr;

	UPTWWeaponData* WData = CurrentItem->GetWeaponData();
	if (WData && WData->AnimMap.Contains(ReloadAnimTag))
	{
		return WData->AnimMap[ReloadAnimTag];
	}

	return nullptr;
}

void UPTWGA_Reload::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (ReloadEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ReloadEffectClass);
		if (SpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
		}
	}
}

void UPTWGA_Reload::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_Reload::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
