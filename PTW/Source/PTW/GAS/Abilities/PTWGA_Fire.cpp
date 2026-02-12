// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Weapon/PTWProjectile.h"
#include "Weapon/PTWWeaponActor.h"
#include "Weapon/PTWWeaponData.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWGA_Fire::UPTWGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(GameplayTags::AbilityBlockTag::Fire);
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
	
	if (UAbilityTask_WaitInputRelease* WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this))
	{
		WaitInputRelease->OnRelease.AddDynamic(this, &UPTWGA_Fire::OnInputReleasedCallback);
		WaitInputRelease->ReadyForActivation();
	}
	
	StartFire();
}

void UPTWGA_Fire::StartFire()
{
	const FPTWFireConext Context = GetFireContext();
	
	if (!Context.IsValid()) return;
	
	if (const UPTWWeaponAttributeSet* AS = Cast<UPTWWeaponAttributeSet>(Context.ASC->GetAttributeSet(WeaponAttributeClass)))
	{
		FireRate = AS->GetFireRate();
	}
	
	AutoFire();
	
	if (!GetWorld()->GetTimerManager().IsTimerActive(AutoFireTimer))
	{
		TWeakObjectPtr<ThisClass> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UPTWGA_Fire::AutoFire ,FireRate, true);
	}
}

void UPTWGA_Fire::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

FPTWFireConext UPTWGA_Fire::GetFireContext() const
{
	FPTWFireConext Context;
	Context.PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	if (Context.PC)
	{
		Context.ASC = Context.PC->GetAbilitySystemComponent();
		if (UPTWInventoryComponent* Inven = Context.PC->GetInventoryComponent())
		{
			Context.WeaponInst = Cast<UPTWWeaponInstance>(Inven->GetCurrentWeaponInst());
		}
	}
	
	return Context;
}

void UPTWGA_Fire::MakeGameplayCue(FPTWGameplayCueMakingInfo Infos)
{
	FGameplayCueParameters Params;
	Params.Instigator = CurrentActorInfo->OwnerActor.Get();
	Params.SourceObject = Infos.Weapon1P; 
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		GameplayTags::GameplayCue::Weapon::Fire, 
		Params
	);
}

void UPTWGA_Fire::AutoFire()
{
	const FPTWFireConext Context = GetFireContext();
	if (!Context.IsValid() || !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		StopFire();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	
	FPTWGameplayCueMakingInfo CueInfos;
	CueInfos.PlayerCharacter = Context.PC;
	CueInfos.Weapon1P = Context.WeaponInst->SpawnedWeapon1P;
	MakeGameplayCue(CueInfos);
	
	EHitType CurrentWeponHitType = Context.WeaponInst->GetWeaponHitType();
	
	if (CurrentWeponHitType == EHitType::HitScan)
	{
		HandleHitScan(Context);
	}
	else if (CurrentWeponHitType == EHitType::Projectile)
	{
		ProjectileTypeFire(Context.PC, Context.WeaponInst);
	}
	
	//캐릭터 반동 함수 호출(박태웅)
	Context.PC->GetWeaponComponent()->ApplyRecoil();
}

void UPTWGA_Fire::PerformLineTrace(FHitResult& HitResult, APTWPlayerCharacter* PlayerCharacter)
{
	APTWPlayerController* Controller = PlayerCharacter->GetController<APTWPlayerController>();
	if (!Controller) return;
	
	FVector StartLoc;
	FRotator Rot;
	Controller->GetPlayerViewPoint(StartLoc, Rot);
	
	FVector End = StartLoc + (Rot.Vector() * MaxRange);
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar); 
	
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc,End, ECC_WeaponAttack, Params);
}

bool UPTWGA_Fire::ValidateHitResult(FHitResult& HitResult)
{
	if (!HitResult.bBlockingHit) return true; 

	AActor* Avatar = GetAvatarActorFromActorInfo();
	
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

void UPTWGA_Fire::ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageGEClass) return;
	
	const FGameplayTag Tag_Damage =  GameplayTags::Data::Damage;
	
	for (auto Data : TargetData.Data)
	{
		const FHitResult* HitResult = Data->GetHitResult();
		AActor* HitActor = HitResult ? HitResult->GetActor() : nullptr;
        
		if (!HitActor) continue;
		
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;
		
		if (!CheckingTag(TargetASC))
		{
			IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(HitActor);
			float CurrentDamage = BaseDamage;
			if (CombatInt)
			{
				CurrentDamage *= CombatInt->GetDamageMultiplier(HitResult->BoneName);
			
				if (HitResult->BoneName == FName("head"))
				{
					CombatInt->ApplyGameplayEffectToSelf(HeadShotEffectClass, 1.0f, MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
				}
			}
		
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(Tag_Damage, -CurrentDamage);
				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, TargetData);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("무적"));
		}
	}
}

void UPTWGA_Fire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopFire();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPTWGA_Fire::OnInputReleasedCallback(float TimeHold)
{ 
	StopFire();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPTWGA_Fire::ProjectileTypeFire(APTWPlayerCharacter* PC, UPTWWeaponInstance* ItemInstance)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 10000.0f);
	FHitResult ScreenHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PC);

	FVector TargetLocation = TraceEnd;
	if (GetWorld()->LineTraceSingleByChannel(ScreenHit, CameraLocation, TraceEnd, ECC_Visibility, Params))
	{
		TargetLocation = ScreenHit.ImpactPoint;
	}


	FVector MuzzleLocation = ItemInstance->SpawnedWeapon3P->GetMuzzleComponent()->GetComponentLocation();
	FVector MuzzleForward = ItemInstance->SpawnedWeapon3P->GetMuzzleComponent()->GetRightVector();
	
	FVector SpawnLocation = MuzzleLocation + (MuzzleForward * 50.0f);
	
	FRotator AdjustedRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = PC;
	SpawnParams.Owner = PC;

	APTWProjectile* Bullet = GetWorld()->SpawnActor<APTWProjectile>(ProjectileClass, SpawnLocation, AdjustedRotation, SpawnParams);
	
	if (Bullet)
	{
		Bullet->DamageSpecHandle = SpecHandle;
	}
}

void UPTWGA_Fire::HandleHitScan(const FPTWFireConext Context)
{
	FHitResult HitResult;
	PerformLineTrace(HitResult, Context.PC);
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	if (ValidateHitResult(HitResult))
	{
		FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);
		float Damage = 0.0f;
		
		if (const UPTWWeaponAttributeSet* AS = Cast<UPTWWeaponAttributeSet>(Context.ASC->GetAttributeSet(WeaponAttributeClass)))
		{
			Damage = AS->GetDamage();
		}
		
		ApplyDamageToTarget(TargetData, Damage);
	}
	ExecuteHitImpactCue(HitResult);
}

void UPTWGA_Fire::ExecuteHitImpactCue(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit && HitResult.GetActor())
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor());
				 	
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		
		if (TargetASC && HitResult.Component->GetCollisionProfileName() == FName("Hit"))
		{
			TargetASC->ExecuteGameplayCue(GameplayTags::GameplayCue::Weapon::HitImpact, CueParams);
		}
		else
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
			if (ASC)
			{
				CueParams.AggregatedSourceTags.AddTag(GameplayTags::GameplayCue::Hit::Wall);
				ASC->ExecuteGameplayCue(GameplayTags::GameplayCue::Weapon::HitImpact, CueParams);
			}
		}
	}
}

bool UPTWGA_Fire::CheckingTag(UAbilitySystemComponent* ASC)
{
	return ASC->HasMatchingGameplayTag(IgnoreTag);
}
