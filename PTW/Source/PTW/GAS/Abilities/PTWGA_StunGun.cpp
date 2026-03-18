// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_StunGun.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWCombatInterface.h"

void UPTWGA_StunGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_StunGun::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;
	
	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (!Data.IsValid()) continue;
		
		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
		if (!HitActor) continue;

		IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(HitActor);
		if (!CombatInterface) return;
		
		CombatInterface->ApplyGameplayEffectToSelf(StunEffect, 1.0f, FGameplayEffectContextHandle());
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
