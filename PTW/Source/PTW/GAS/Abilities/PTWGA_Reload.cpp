// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Reload.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "CoreFramework/PTWPlayerCharacter.h"
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

	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = nullptr;
	UPTWInventoryComponent* Inven = PC->GetInventoryComponent();

	if (Inven)
	{
		if (UPTWItemInstance* CurrentItem = Inven->GetCurrentWeaponInst())
		{
			if (UPTWWeaponData* WData = CurrentItem->GetWeaponData())
			{
				if (WData->AnimMap.Contains(ReloadAnimTag))
				{
					MontageToPlay = *WData->AnimMap.Find(ReloadAnimTag);
				}
			}
		}
	}

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

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		NAME_None,
		false
	);

	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

		MontageTask->ReadyForActivation();
	}

	PC->PlayMontage1P(MontageToPlay);

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.ReloadRefill"));

	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		EventTag,
		nullptr,
		false,
		false
	);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGameplayEventReceived);
		WaitEventTask->ReadyForActivation();
	}
}

void UPTWGA_Reload::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (ReloadEffectClass)
	{
		ApplyGameplayEffectToOwner(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			ReloadEffectClass.GetDefaultObject(),
			1.0f
		);
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
