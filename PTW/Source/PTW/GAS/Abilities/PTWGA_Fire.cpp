// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"

UPTWGA_Fire::UPTWGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Reload")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Sprinting")));
}

void UPTWGA_Fire::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	StartFire(Handle, ActorInfo, ActivationInfo);
}

void UPTWGA_Fire::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	StopFire();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	AutoFire(Handle, ActorInfo, ActivationInfo);
}

void UPTWGA_Fire::StartFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo)
{
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
	
	if (!CurrentInst || CurrentInst->CurrentAmmo <= 0)
	{
		return;
	}
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		ApplyCost(Handle, ActorInfo, ActivationInfo);
	}
	
	FPTWGameplayCueMakingInfo Infos;
	Infos.PlayerCharacter = PC;
	Infos.ItemInstance = CurrentInst;
	
	MakeGameplayCue(Handle, ActorInfo, ActivationInfo, Infos);
	
	// Server-side Validation 피격 처리
	FHitResult HitResult;
	PerformLineTrace(HitResult, PC);
	
	FGameplayAbilityTargetDataHandle TargetData; 
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewData->HitResult = HitResult;
	TargetData.Add(NewData);
	
	if (HasAuthority(&ActivationInfo))
	{
		// [Server-side Validation]
		if (ValidateHitResult(HitResult)) // 검증 함수 호출
		{
			UE_LOG(LogTemp, Warning, TEXT("Take Damage Actor Name : %s"), *HitResult.GetActor()->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Hit Bone : %s"), *HitResult.BoneName.ToString());
			
			// 검증 성공 시 데미지 GE 적용
			ApplyDamageToTarget(Handle, ActorInfo, ActivationInfo, TargetData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Hit Detected! (Cheat or Lag)"));
		}
	}
	
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Fire::PerformLineTrace(FHitResult& HitResult, APTWPlayerCharacter* PlayerCharacter)
{
	APTWPlayerController* Controller = PlayerCharacter->GetController<APTWPlayerController>();
	if (!Controller) return;
	
	FVector StartLoc;
	FRotator Rot;
	Controller->GetPlayerViewPoint(StartLoc, Rot);
	
	FVector End = StartLoc + (Rot.Vector() * 5000.0f);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc,End, ECC_WeaponAttack);
	
	DrawDebugLine(GetWorld(), StartLoc, End, FColor::Green, false, 10.0f);
}

bool UPTWGA_Fire::ValidateHitResult(FHitResult& HitResult)
{
	if (!HitResult.bBlockingHit) return true; // 아무것도 안 맞은 건 검증 필요 없음

	AActor* Avatar = GetAvatarActorFromActorInfo();
	float MaxRange = 5000.f; // 무기 데이터에서 가져온 사거리

	// 검증 1: 거리 체크 (너무 먼 곳을 맞췄는가?)
	float Distance = FVector::Dist(Avatar->GetActorLocation(), HitResult.ImpactPoint);
	if (Distance > MaxRange + 100.f) return false; 

	// 검증 2: 시야 체크 (벽 뒤에 있는 적을 맞췄는가?)
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	FHitResult ValidationHit;
    
	// 서버에서 다시 한번 짧게 쏴서 중간에 벽이 없는지 확인
	GetWorld()->LineTraceSingleByChannel(ValidationHit, Avatar->GetActorLocation(), HitResult.ImpactPoint, ECC_Visibility, Params);
    
	if (ValidationHit.bBlockingHit && ValidationHit.GetActor() != HitResult.GetActor())
	{
		// 실제 맞은 적보다 앞에 다른 장애물이 있다면 무효
		return false;
	}

	return true;
}

void UPTWGA_Fire::ApplyDamageToTarget(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	if (!DamageGEClass) return;
	
	for (auto Data : TargetData.Data)
	{
		FHitResult* HitResult = const_cast<FHitResult*>(Data->GetHitResult());
		if (HitResult && HitResult->GetActor())
		{
			// 4. EffectSpec 생성 (서버에서 생성해야 안전함)
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());

			// 5. 필요시 데이터 주입 (예: 데미지 수치, 히트 위치 등)
			// SpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag_Damage, 50.f);

			// 6. 대상의 ASC를 찾아 GE 적용
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult->GetActor());
			if (TargetASC)
			{
				ApplyGameplayEffectSpecToTarget(Handle,ActorInfo, ActivationInfo, SpecHandle, TargetData);
			}
		}
	}
}
