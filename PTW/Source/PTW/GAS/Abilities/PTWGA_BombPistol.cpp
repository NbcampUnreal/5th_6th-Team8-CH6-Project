// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWGA_BombPistol.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/Item/BombItem/PTWBombActor.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "System/PTWItemSpawnManager.h"

void UPTWGA_BombPistol::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_BombPistol::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;

	AActor* MyActor = GetAvatarActorFromActorInfo();
	if (!MyActor) return;
	
	AActor* AttachingActor = nullptr;
	TArray<AActor*> AttachedActors;
	MyActor->GetAttachedActors(AttachedActors);

	for (AActor* Attached : AttachedActors)
	{
		if (Attached && Attached->IsA(APTWBombActor::StaticClass()))
		{
			AttachingActor = Attached;
			break;
		}
	}
	if (!AttachingActor) return;

	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (!Data.IsValid()) continue;

		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
		if (!HitActor) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;

		// ReflectShield

		if (ConsumeReflectShieldIfAny(HitActor))
		{
			break;
		}
		
		AttachBombToTarget(AttachingActor, HitActor);
		ProcessItemTransfer(MyActor, HitActor);
		break;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_BombPistol::AttachBombToTarget(AActor* Bomb, AActor* Target)
{
	if (USceneComponent* TargetMesh = Target->FindComponentByClass<USkeletalMeshComponent>())
	{
		Bomb->AttachToComponent(
			TargetMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName(TEXT("BombHeadSocket"))
		);
		Bomb->SetOwner(Target);
	}
}

void UPTWGA_BombPistol::ProcessItemTransfer(AActor* Source, AActor* Target)
{
	if (APTWPlayerCharacter* TargetPC = Cast<APTWPlayerCharacter>(Target))
	{
		GiveItemAndEquip(TargetPC);
	}

	if (APTWPlayerCharacter* SourcePC = Cast<APTWPlayerCharacter>(Source))
	{
		RemoveSourceWeapon(SourcePC);
	}
}

void UPTWGA_BombPistol::GiveItemAndEquip(APTWPlayerCharacter* TargetPC)
{
	if (!TargetPC) return;

	if (IPTWCombatInterface* TargetCombatInt = Cast<IPTWCombatInterface>(TargetPC))
	{
		TargetCombatInt->ApplyGameplayEffectToSelf(BombPistolEffect, 1.0f, FGameplayEffectContextHandle());
	}

	if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWPlayerState* PS = TargetPC->GetPlayerState<APTWPlayerState>())
		{
			SpawnManager->SpawnSingleItem(PS, ItemDef);
		}
	}

	if (UPTWInventoryComponent* Inven = TargetPC->GetInventoryComponent())
	{
		if (TargetPC->HasAuthority())
		{
			Inven->EquipWeapon(0);
		}
	}
}

void UPTWGA_BombPistol::RemoveSourceWeapon(APTWPlayerCharacter* SourcePC)
{
	if (IPTWCombatInterface* TargetCombatInt = Cast<IPTWCombatInterface>(SourcePC))
	{
		TargetCombatInt->RemoveEffectWithTag(GameplayTags::State::Slowing);
	}

	if (UPTWInventoryComponent* Inven = SourcePC->GetInventoryComponent())
	{
		Inven->RemoveWeaponItem();
	}
}

// ReflectShield
bool UPTWGA_BombPistol::ConsumeReflectShieldIfAny(AActor* TargetActor) const
{
	if (!TargetActor) return false;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return false;
	
	if (!TargetASC->HasMatchingGameplayTag(GameplayTags::State::Passive::ReflectShield))
	{
		return false;
	}
	
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(GameplayTags::State::Passive::ReflectShield);

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer);
	TArray<FActiveGameplayEffectHandle> Handles = TargetASC->GetActiveEffects(Query);

	if (Handles.Num() <= 0)
	{
		return false;
	}
	
	TargetASC->RemoveActiveGameplayEffect(Handles[0]);
	return true;
}
