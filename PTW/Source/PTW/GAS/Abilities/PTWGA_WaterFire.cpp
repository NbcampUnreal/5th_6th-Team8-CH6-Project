// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_WaterFire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWCombatInterface.h"

void UPTWGA_WaterFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_WaterFire::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;
	
	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (!Data.IsValid()) continue;

		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
		
		if (!HitActor) continue;

		if (IPTWCombatInterface* TargetCombatInterface = Cast<IPTWCombatInterface>(HitActor))
		{
			TargetCombatInterface->ApplyGameplayEffectToSelf(WaterFireEffect, 1.0f, FGameplayEffectContextHandle());
		}
		
		break; 
	}
}



