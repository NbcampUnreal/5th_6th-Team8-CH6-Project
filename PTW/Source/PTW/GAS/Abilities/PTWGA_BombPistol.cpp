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
	 FGameplayTag TransferredTag = FGameplayTag::RequestGameplayTag(FName("Event.MiniGame.BombTransferred"));
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
	 		float TimeToTransfer = CalculateBombTimer();
	 		
	 		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	 		TargetASC->RemoveActiveEffectsWithGrantedTags(BombTag.GetSingleTagContainer());
	 		TargetCombatInt->ApplyGameplayEffectWithDuration(BombPistolEffect, 1.0f,TimeToTransfer,ContextHandle);
	 		ASC->AddLooseGameplayTag(TransferredTag);
	 		ASC->RemoveActiveEffectsWithGrantedTags(BombTag.GetSingleTagContainer());
	 		break;
	 	}
	 }

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
}

float UPTWGA_BombPistol::CalculateBombTimer()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayTag BombTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Bomb"));
	
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(BombTag.GetSingleTagContainer());
	TArray<float> RemainingTimes = ASC->GetActiveEffectsTimeRemaining(Query);
	
	float TimeToTransfer = 10.0f;
	
	if (RemainingTimes.Num() > 0)
	{
		RemainingTimes.Sort();
		TimeToTransfer = RemainingTimes[0];
	}
	return TimeToTransfer;
}




