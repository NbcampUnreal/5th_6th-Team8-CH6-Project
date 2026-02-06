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
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (!Data.IsValid()) continue;

		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
        
		if (!HitActor) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;
		
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
	if (UPTWInventoryComponent* Inven = SourcePC->GetInventoryComponent())
	{
		Inven->RemoveWeaponItem();
	}
}






