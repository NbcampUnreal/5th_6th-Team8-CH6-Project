// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_BombPistol.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "MiniGame/Item/PTWBombActor.h"

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
	 	AActor* MyActor = GetAvatarActorFromActorInfo();
	 	
	 	if (!HitActor && !MyActor) continue;
	
	 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	 	if (!TargetASC) continue;
	 	
	 	// 맞춘 상대의 머리에 내 머리에 있는 폭탄 Attach 
	 	TArray<AActor*> AttachedActors;
	 	MyActor->GetAttachedActors(AttachedActors);
	 	
	 	AActor* AttachingActor = nullptr;
	 	
	 	for (AActor* AttachedActor : AttachedActors)
	 	{
	 		if (AttachedActor->IsA(APTWBombActor::StaticClass()))
	 		{
	 			AttachingActor = AttachedActor;
	 			break;
	 		}
	 	}
	 	
	 	if (AttachingActor && HitActor)
	 	{
	 		USceneComponent* TargetMesh = HitActor->FindComponentByClass<USkeletalMeshComponent>();
    
	 		if (TargetMesh)
	 		{
	 			AttachingActor->AttachToComponent(
					 TargetMesh, 
					 FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					 FName(TEXT("BombHeadSocket"))
				 );
	 			AttachingActor->SetOwner(HitActor);
	 		}
	 	}}
	 	
	 	// IPTWCombatInterface* TargetCombatInt = Cast<IPTWCombatInterface>(HitActor);
	 	// IPTWCombatInterface* CombatIntMine = Cast<IPTWCombatInterface>(CurrentActorInfo->AvatarActor.Get()); 
	 	//
	 	// if (TargetCombatInt && CombatIntMine)
	 	// {
	 	// 	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	 	// 	TargetCombatInt->ApplyGameplayEffectToSelf(BombPistolEffect, 1.0f,ContextHandle); // 타겟에게 Effect 부여
	 	// 	CombatIntMine->RemoveEffectWithTag(BombTag);
	 	// 	break;
	 	// }
	 	
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}





