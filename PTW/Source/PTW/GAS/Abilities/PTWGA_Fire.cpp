// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"

UPTWGA_Fire::UPTWGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Reload")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Sprinting")));
}

void UPTWGA_Fire::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	StopFire();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitCheck(Handle,ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	StartFire(Handle, ActorInfo, ActivationInfo);
}

void UPTWGA_Fire::StartFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo)
{
	AutoFire(Handle, ActorInfo, ActivationInfo);
	
	if (!GetWorld()->GetTimerManager().IsTimerActive(AutoFireTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, FTimerDelegate::CreateLambda([Handle,ActorInfo,ActivationInfo, this]()
		{
			AutoFire(Handle, ActorInfo, ActivationInfo);
		}),FireRate, true);
	}
}

void UPTWGA_Fire::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void UPTWGA_Fire::MakeGameplayCue(const FGameplayAbilitySpecHandle Handle, 
                                  const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                  FPTWGameplayCueMakingInfo Infos)
{
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(FireEffectClass);
    
	if (SpecHandle.IsValid())
	{
		
		FGameplayEffectContextHandle Context = SpecHandle.Data->GetContext();
		if (Infos.PlayerCharacter)
		{
			if (Infos.PlayerCharacter->IsLocallyControlled())
			{
				Context.AddSourceObject(Infos.ItemInstance->SpawnedWeapon1P);
			}
			else
			{
				Context.AddSourceObject(Infos.ItemInstance->SpawnedWeapon3P);
			}
		}
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

void UPTWGA_Fire::AutoFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo)
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	UPTWInventoryComponent* Inven = PC->FindComponentByClass<UPTWInventoryComponent>();
	if (!Inven) return;
	
	UPTWItemInstance* CurrentInst = Inven->GetCurrentWeaponInst();
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		StopFire();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//ApplyCost(Handle, ActorInfo, ActivationInfo);
	
	FPTWGameplayCueMakingInfo Infos;
	Infos.PlayerCharacter = PC;
	Infos.ItemInstance = CurrentInst;
	
	MakeGameplayCue(Handle, ActorInfo, ActivationInfo, Infos);
	
	FHitResult HitResult;
	PerformLineTrace(HitResult, PC);
	
	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);
	
	// Server-side Validation 피격 처리
	if (HasAuthority(&ActivationInfo))
	{
		if (ValidateHitResult(HitResult)) 
		{
			float Damage = CurrentInst->SpawnedWeapon1P->GetWeaponData()->BaseDamage;
			ApplyDamageToTarget(Handle, ActorInfo, ActivationInfo, TargetData, Damage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Hit Detected! (Cheat or Lag)"));
		}
	}
	
	//캐릭터 반동 함수 호출(박태웅)
	PC->ApplyRecoil();
	
	//EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Fire::PerformLineTrace(FHitResult& HitResult, APTWPlayerCharacter* PlayerCharacter)
{
	APTWPlayerController* Controller = PlayerCharacter->GetController<APTWPlayerController>();
	if (!Controller) return;
	
	FVector StartLoc;
	FRotator Rot;
	Controller->GetPlayerViewPoint(StartLoc, Rot);
	
	FVector End = StartLoc + (Rot.Vector() * 5000.0f);
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar); 
	
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc,End, ECC_WeaponAttack, Params);
	
	DrawDebugLine(GetWorld(), StartLoc, End, FColor::Green, false, 10.0f);
}

bool UPTWGA_Fire::ValidateHitResult(FHitResult& HitResult)
{
	if (!HitResult.bBlockingHit) return true; 

	AActor* Avatar = GetAvatarActorFromActorInfo();
	float MaxRange = 5000.f; 
	
	float Distance = FVector::Dist(Avatar->GetActorLocation(), HitResult.ImpactPoint);
	if (Distance > MaxRange + 100.f) return false; 

	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	FHitResult ValidationHit;
    

	GetWorld()->LineTraceSingleByChannel(ValidationHit, Avatar->GetActorLocation(), HitResult.ImpactPoint, ECC_Visibility, Params);
    
	if (ValidationHit.bBlockingHit && ValidationHit.GetActor() != HitResult.GetActor())
	{
		return false;
	}

	return true;
}

void UPTWGA_Fire::ApplyDamageToTarget(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	if (!DamageGEClass) return;
	
	for (auto Data : TargetData.Data)
	{
		FHitResult* HitResult = const_cast<FHitResult*>(Data->GetHitResult());
		if (HitResult && HitResult->GetActor())
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
			FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
			
			if (HitResult->BoneName == FName("head"))
			{
				BaseDamage *= 2.0f;
			}
			
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag_Damage, -BaseDamage);
			
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult->GetActor());
			if (TargetASC)
			{
				ApplyGameplayEffectSpecToTarget(Handle,ActorInfo, ActivationInfo, SpecHandle, TargetData);
			}
		}
	}
}
