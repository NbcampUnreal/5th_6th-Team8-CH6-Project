// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_BombPistol.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/Item/PTWBombActor.h"
#include "System/PTWItemSpawnManager.h"

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
	AActor* HitActor = nullptr;
	AActor* MyActor = nullptr;
	
	for (auto Data : TargetData.Data)
	{
	 	const FHitResult* HitResult = Data->GetHitResult();
	 	HitActor = HitResult ? HitResult->GetActor() : nullptr;
	 	MyActor = GetAvatarActorFromActorInfo();
	 	
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
	 		
	 		UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	
	 		if (SpawnManager && HitActor)
	 		{
	 			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(HitActor))
	 			{
	 				APTWPlayerState* PS = Cast<APTWPlayerState>(PC->GetPlayerState());
	 				SpawnManager->SpawnSingleItem(PS, ItemDef);
	 			}
	 		}
	
	 		if (MyActor)
	 		{
	 			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyActor))
	 			{
	 				if (UPTWInventoryComponent* Inven = PC->GetInventoryComponent())
	 				{
	 					Inven->RemoveWeaponItem();
	 				}
	 			}
	 		}
	 	}
	}
	
	
	
	 	
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





