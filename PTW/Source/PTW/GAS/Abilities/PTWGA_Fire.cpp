// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWProjectile.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"
#include "Kismet/KismetMathLibrary.h"

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
	
	UAbilityTask_WaitInputRelease* WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputRelease->OnRelease.AddDynamic(this, &UPTWGA_Fire::OnInputReleasedCallback);
	WaitInputRelease->ReadyForActivation();
	
	StartFire(Handle, ActorInfo, ActivationInfo);
}

void UPTWGA_Fire::StartFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo)
{
	AutoFire(Handle, ActorInfo, ActivationInfo);
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PC);
	if (!ASC) return;
	
	const UPTWWeaponAttributeSet* AS = Cast<UPTWWeaponAttributeSet>(ASC->GetAttributeSet(WeaponAttributeClass));
	if (AS)
	{
		FireRate = AS->GetFireRate();
	}
	
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
	FGameplayCueParameters Params;
	Params.Instigator = ActorInfo->OwnerActor.Get();
	Params.SourceObject = Infos.Weapon1P; 
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Fire")), 
		Params
	);
}

void UPTWGA_Fire::AutoFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo)
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	UPTWInventoryComponent* Inven = PC->FindComponentByClass<UPTWInventoryComponent>();
	if (!Inven) return;
	
	UPTWItemInstance* CurrentInst = Inven->GetCurrentWeaponInst();
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PC);
	if (!ASC) return;
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		StopFire();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	

	FPTWGameplayCueMakingInfo Infos;
	Infos.PlayerCharacter = PC;
	Infos.Weapon1P = CurrentInst->SpawnedWeapon1P;
	
	MakeGameplayCue(Handle, ActorInfo, ActivationInfo, Infos);
	
	if (CurrentInst->GetWeaponHitType() == EHitType::HitScan)
	{
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
				
				 if (HitResult.bBlockingHit && HitResult.GetActor())
				 {
				 	FName ProfileName = HitResult.Component->GetCollisionProfileName();
				 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor());
				 	
				 	if (ProfileName.IsEqual(FName("Hit")))
				 	{
				 		FGameplayCueParameters CueParams;
				 		CueParams.Location = HitResult.ImpactPoint;
				 		CueParams.Normal = HitResult.ImpactNormal;
				 		
				 		TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.HitImpact")), CueParams);
				 	}
				 }
			}
		}
	}
	else if (CurrentInst->GetWeaponHitType() == EHitType::Projectile)
	{
		ProjectileTypeFire(PC, CurrentInst);
	}
	
	//캐릭터 반동 함수 호출(박태웅)
	PC->ApplyRecoil();
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
		const FHitResult* HitResult = Data->GetHitResult();
		if (HitResult && HitResult->GetActor())
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
			FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
			
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult->GetActor());
			if (HitResult->BoneName == FName("head"))
			{
				BaseDamage *= 2.0f;
				
				//FIXME : 테스트 용도// 후에 GE로 변경하거나, 해당 코드를 그대로 사용 하되, 애니메이션 몽타주가 끝날 때 해당 태그 제거 하는 방식으로 변경 예정
				FGameplayTag HeadShotTag = FGameplayTag::RequestGameplayTag(FName("State.HitReaction.HeadShot"));
				TargetASC->AddLooseGameplayTag(HeadShotTag);
			}
			
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag_Damage, -BaseDamage);
			
			if (TargetASC)
			{
				ApplyGameplayEffectSpecToTarget(Handle,ActorInfo, ActivationInfo, SpecHandle, TargetData);
			}
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

void UPTWGA_Fire::HitScanTypeFire(APTWPlayerCharacter* PC)
{
	
}

void UPTWGA_Fire::ProjectileTypeFire(APTWPlayerCharacter* PC, UPTWItemInstance* ItemInstance)
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
