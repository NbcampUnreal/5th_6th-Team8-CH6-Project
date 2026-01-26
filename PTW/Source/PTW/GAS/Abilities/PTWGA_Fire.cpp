// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Fire.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
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
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(FireEffectClass);
    
	 if (SpecHandle.IsValid())
	 {
	 	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	 	if (!PC) return;
	
	 	UPTWInventoryComponent* Inven = PC->FindComponentByClass<UPTWInventoryComponent>();
	 	if (!Inven) return;
	 	
	 	UPTWItemInstance* CurrentInst = Inven->GetCurrentWeaponInst();
	 	
	 	FGameplayEffectContextHandle Context = SpecHandle.Data->GetContext();
        if (PC)
        {
            if (PC->IsLocallyControlled())
            {
                Context.AddSourceObject(CurrentInst->SpawnedWeapon1P);
            }
            else
            {
                Context.AddSourceObject(CurrentInst->SpawnedWeapon3P);
            }
        }
	 	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	 }
	
	
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
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	UPTWInventoryComponent* Inven = PC->FindComponentByClass<UPTWInventoryComponent>();
	if (!Inven) return;
	
	UPTWItemInstance* CurrentInst = Inven->GetCurrentWeaponInst();
	
	if (CurrentInst->CurrentAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Ammo!"));
		return;
	}
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		CurrentInst->CurrentAmmo--;
	}
	
	
	
	// if (CurrentInst->SpawnedWeapon)
	// {
	// 	FGameplayCueParameters CueParams;
	// 	CueParams.Instigator = CurrentInst->SpawnedWeapon;
 //        
	// 	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	// 	ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Fire")), CueParams);
	// }
}
