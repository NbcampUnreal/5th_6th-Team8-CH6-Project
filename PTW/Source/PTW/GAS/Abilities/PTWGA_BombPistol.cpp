// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_BombPistol.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "CoreFramework/PTWCombatInterface.h"

void UPTWGA_BombPistol::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bIsTransferred = false;
	
	FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	
	UAbilityTask_WaitGameplayTagRemoved* WaitTagRemovedTask = UAbilityTask_WaitGameplayTagRemoved::
	WaitGameplayTagRemove(
		this, 
		BombTag
	);
	
	if (WaitTagRemovedTask)
	{
		WaitTagRemovedTask->Removed.AddDynamic(this, &UPTWGA_BombPistol::OnBombTagRemoved);
		WaitTagRemovedTask->ReadyForActivation();
	}
}

void UPTWGA_BombPistol::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;
	
	for (auto Data : TargetData.Data)
	{
		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
        
		if (!HitActor) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;
		
		IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(HitActor);
		if (CombatInt)
		{
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			CombatInt->ApplyGameplayEffectToSelf(BombPistolEffect, 1.0f, ContextHandle);
			bIsTransferred = true;
		}
	}
}

void UPTWGA_BombPistol::OnBombTagRemoved()
{
	if (!bIsTransferred)
	{
		// 데미지 부여
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
