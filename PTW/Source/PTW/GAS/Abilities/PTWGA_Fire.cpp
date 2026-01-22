// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWWeaponActor.h"

UPTWGA_Fire::UPTWGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPTWGA_Fire::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	StartFire();
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
	AutoFire();
	//FIXME: 테스트 임시 코드
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_Fire::StartFire()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(AutoFireTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UPTWGA_Fire::AutoFire, FireRate, true);
	}
}

void UPTWGA_Fire::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void UPTWGA_Fire::AutoFire()
{
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (APTWBaseCharacter* BaseCharacter = Cast<APTWBaseCharacter>(OwnerActor))
	{
		if (UPTWInventoryComponent* InvenComp = BaseCharacter->FindComponentByClass<UPTWInventoryComponent>())
		{
			if (APTWWeaponActor* Weapon = InvenComp->GetCurrentWeaponActor())
			{
				FGameplayCueParameters CueParams;
				CueParams.Instigator = Weapon;
				
				UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
				ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Fire")), CueParams);
				UE_LOG(LogTemp, Warning, TEXT("TEST WEAPON"));
			}
		}
	}
}
