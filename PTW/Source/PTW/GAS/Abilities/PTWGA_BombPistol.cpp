// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_BombPistol.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWCombatInterface.h"

void UPTWGA_BombPistol::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, CurrentActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UPTWGA_BombPistol::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	 if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;
	
	 FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	 UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	 for (auto Data : TargetData.Data)
	 {
	 	const FHitResult* HitResult = Data->GetHitResult();
	 	AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
	 	
	 	if (!HitActor) continue;
	
	 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	 	if (!TargetASC) continue;
	 	
	 	IPTWCombatInterface* TargetCombatInt = Cast<IPTWCombatInterface>(HitActor);
	 	IPTWCombatInterface* CombatIntMine = Cast<IPTWCombatInterface>(CurrentActorInfo->AvatarActor.Get()); 
	 	
	 	if (TargetCombatInt && CombatIntMine)
	 	{
	 		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	 		TargetCombatInt->ApplyGameplayEffectToSelf(BombPistolEffect, 1.0f,ContextHandle); // 타겟에게 Effect 부여
	 		CombatIntMine->RemoveEffectWithTag(BombTag);
	 		break;
	 	}
	 }

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
}




