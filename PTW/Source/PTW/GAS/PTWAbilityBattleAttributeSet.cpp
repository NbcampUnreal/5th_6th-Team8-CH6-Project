// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PTWAbilityBattleAttributeSet.h"

#include "Net/UnrealNetwork.h"

UPTWAbilityBattleAttributeSet::UPTWAbilityBattleAttributeSet()
{
	
}

void UPTWAbilityBattleAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, Armor, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, HealthRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, LifeSteal, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, LifeOnHit, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, Shield, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, MaxShield, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, DamageReceived, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, ReflectDamagePercent, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, HealPower, COND_None, REPNOTIFY_OnChanged);
}

void UPTWAbilityBattleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UPTWAbilityBattleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UPTWAbilityBattleAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UPTWAbilityBattleAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, Armor, OldArmor);
}

void UPTWAbilityBattleAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, HealthRegen, OldHealthRegen);
}

void UPTWAbilityBattleAttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, LifeSteal, OldLifeSteal);
}

void UPTWAbilityBattleAttributeSet::OnRep_LifeOnHit(const FGameplayAttributeData& OldLifeOnHit)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, LifeOnHit, OldLifeOnHit);
}

void UPTWAbilityBattleAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, Shield, OldShield);
}

void UPTWAbilityBattleAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, MaxShield, OldMaxShield);
}

void UPTWAbilityBattleAttributeSet::OnRep_DamageReceived(const FGameplayAttributeData& OldDamageReceived)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, DamageReceived, OldDamageReceived);
}

void UPTWAbilityBattleAttributeSet::OnRep_ReflectDamagePercent(const FGameplayAttributeData& OldReflectDamagePercent)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, ReflectDamagePercent, OldReflectDamagePercent);
}

void UPTWAbilityBattleAttributeSet::OnRep_HealPower(const FGameplayAttributeData& OldHealPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, HealPower, OldHealPower);
}
