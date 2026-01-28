// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWWeaponAttributeSet.h"

#include "Net/UnrealNetwork.h"

UPTWWeaponAttributeSet::UPTWWeaponAttributeSet()
{
	
}

void UPTWWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWWeaponAttributeSet, MaxAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWWeaponAttributeSet, CurrentAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWWeaponAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWWeaponAttributeSet, FireRate, COND_None, REPNOTIFY_Always);
}

void UPTWWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetCurrentAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0, GetMaxAmmo());
	}
}

void UPTWWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	
}

void UPTWWeaponAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetCurrentAmmoAttribute())
	{
		//SetCurrentAmmo(FMath::Clamp(GetCurrentAmmo(), 0, GetMaxAmmo()));
	}
}

void UPTWWeaponAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWWeaponAttributeSet, MaxAmmo, OldMaxAmmo);
}

void UPTWWeaponAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWWeaponAttributeSet, Damage, OldDamage);
}

void UPTWWeaponAttributeSet::OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWWeaponAttributeSet, CurrentAmmo, OldCurrentAmmo);
}

void UPTWWeaponAttributeSet::OnRep_FireRate(const FGameplayAttributeData& OldFireRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWWeaponAttributeSet, FireRate, OldFireRate);
}
