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
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
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
		PlayEmptyClickCue();
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

bool UPTWGA_Fire::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		if (!CheckCost(Handle, ActorInfo, OptionalRelevantTags))
		{
			return true;
		}
		return false;
	}
	
	return true;
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
			Context.WeaponInst = Inven->GetCurrentWeaponInst<UPTWWeaponInstance>();
		}
	}
	
	return Context;
}

float UPTWGA_Fire::CalculateDamage(const FPTWFireConext Context)
{
	if (const UPTWWeaponAttributeSet* AS = Cast<UPTWWeaponAttributeSet>(Context.ASC->GetAttributeSet(WeaponAttributeClass)))
	{
		return AS->GetDamage();
	}

	return 0.0f;
}

void UPTWGA_Fire::MakeGameplayCue(FPTWGameplayCueMakingInfo Infos, FGameplayTag ExecuteTag)
{
	FGameplayCueParameters Params;
	Params.Instigator = CurrentActorInfo->OwnerActor.Get();
	Params.SourceObject = Infos.Weapon1P;
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		ExecuteTag, 
		Params
	);
}

void UPTWGA_Fire::AutoFire()
{
	const FPTWFireConext Context = GetFireContext();
	if (!Context.IsValid() || !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		PlayEmptyClickCue();
		StopFire();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	
	FPTWGameplayCueMakingInfo CueInfos;
	CueInfos.PlayerCharacter = Context.PC;
	CueInfos.Weapon1P = Context.WeaponInst->SpawnedWeapon1P;
	MakeGameplayCue(CueInfos, GameplayTags::GameplayCue::Weapon::Fire);
	
	EHitType CurrentWeponHitType = Context.WeaponInst->GetWeaponHitType();
	
	if (CurrentWeponHitType == EHitType::HitScan)
	{
		HandleHitScan(Context);
	}
	else if (CurrentWeponHitType == EHitType::Projectile)
	{
		ProjectileTypeFire(Context.PC, Context.WeaponInst);
	}
	
	FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Anim.Fire"));
	Context.PC->GetWeaponComponent()->PlayWeaponMontages(FireTag);
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
	
	float SweepRad = 35.0f;
	
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SweepRad);
	
	GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLoc,
		End,
		FQuat::Identity,
		ECC_WeaponAttack,
		SphereShape,
		Params
		);
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
		
		if (!CheckingTag(TargetASC) && CheckingTeam(HitActor))
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

			if (HitResult->bBlockingHit)
			{
				UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
				if (MyASC)
				{
					FGameplayCueParameters CueParams;
					CueParams.Instigator = GetAvatarActorFromActorInfo();
					CueParams.Location = HitResult->ImpactPoint;

					if (HitResult->BoneName == FName("head"))
					{
						CueParams.AggregatedSourceTags.AddTag(
							GameplayTags::State::HitReaction_HeadShot
						);
					}

					MyASC->ExecuteGameplayCue(
						GameplayTags::GameplayCue::Hit::Confirm,
						CueParams
					);
				}
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
	PerformLineTrace(HitResult, Context.PC); // 클라이언트 Hit
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	if (ValidateHitResult(HitResult)) //서버 검증 Server-Side Validation
	{
		FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);
		float Damage = 0.0f;
		
		Damage = CalculateDamage(Context);
		
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

bool UPTWGA_Fire::CheckingTeam(AActor* TargetActor)
{
	APTWPlayerCharacter* TargetPC = Cast<APTWPlayerCharacter>(TargetActor);
	APTWPlayerCharacter* MyPC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	if (!TargetPC || !MyPC) return true;

	APTWPlayerState* TargetPS = Cast<APTWPlayerState>(TargetPC->GetPlayerState());
	APTWPlayerState* MyPS = Cast<APTWPlayerState>(MyPC->GetPlayerState());
	
	if (!TargetPS || !MyPS) return true;
	if (TargetPS->GetPlayerRoundData().TeamId == -1) return true; // 개인전인 경우
	if (TargetPS->GetPlayerRoundData().TeamId == MyPS->GetPlayerRoundData().TeamId) return false; //같은 팀인 경우
	
	return true;
}

void UPTWGA_Fire::PlayEmptyClickCue()
{
	FPTWGameplayCueMakingInfo Infos;
	FPTWFireConext Context = GetFireContext();
	Infos.PlayerCharacter = Context.PC;
	if (!Context.WeaponInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayEmptyClickCue: WeaponInst is NULL!"));
		return; 
	}
	Infos.Weapon1P = Context.WeaponInst->SpawnedWeapon1P;
	MakeGameplayCue(Infos, GameplayTags::GameplayCue::Weapon::Empty);
}
