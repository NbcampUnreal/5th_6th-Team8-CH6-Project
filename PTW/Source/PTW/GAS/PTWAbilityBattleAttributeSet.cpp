// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PTWAbilityBattleAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWAbilityBattleAttributeSet::UPTWAbilityBattleAttributeSet()
{
	
}

void UPTWAbilityBattleAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, Armor, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, HealthRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, ShieldRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, LifeSteal, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, LifeStealOnHit, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, Shield, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, MaxShield, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, DamageReceived, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, ReflectDamagePercent, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, HealPower, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAbilityBattleAttributeSet, DamageMultiplier, COND_None, REPNOTIFY_OnChanged);
}

void UPTWAbilityBattleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	const UPTWAttributeSet* Set = ASC->GetSet<UPTWAttributeSet>();
	if (!Set) return;

	if (Set->GetHealth() <= 0.f || Attribute == GetShieldAttribute())
	{
		NewValue = GetShield();
	}
}

void UPTWAbilityBattleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMaxShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}

	if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}
}

void UPTWAbilityBattleAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
}

void UPTWAbilityBattleAttributeSet::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	float OldValue = Data.OldValue;
	float NewValue = Data.NewValue;
	
	if (Data.OldValue <= 0.f) return;
	
	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(GetOwningAbilitySystemComponent());
	if (!ASC) return;
	
	float Delta = NewValue - OldValue;
	
	ASC->ApplyModToAttribute(
		UPTWAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		Delta
		);
	
}

void UPTWAbilityBattleAttributeSet::HandleDamaged(UAbilitySystemComponent* Target, UAbilitySystemComponent* Source, float Damage)
{
	if (!Source || !Target) return;

	if (Damage <= 0.f) return;
	
	const UPTWAbilityBattleAttributeSet* SourceSet = Source->GetSet<UPTWAbilityBattleAttributeSet>();
	if (!SourceSet) return;
	
	AActor* SourceActor = Source->GetAvatarActor();
	if (!SourceActor || !SourceActor->HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	bCanHealthRegen = false;
	bCanShieldRegen = false;

	World->GetTimerManager().ClearTimer(HealthRegenLoopTimer);
	World->GetTimerManager().ClearTimer(ShieldRegenLoopTimer);
		
	World->GetTimerManager().ClearTimer(HealthRegenTimer);
	World->GetTimerManager().ClearTimer(ShieldRegenTimer);
	
	World->GetTimerManager().SetTimer(HealthRegenTimer, this, &UPTWAbilityBattleAttributeSet::StartHealthRegen, HealthRegenDelay);
	World->GetTimerManager().SetTimer(ShieldRegenTimer, this, &UPTWAbilityBattleAttributeSet::StartShieldRegen, ShieldRegenDelay);
	
	const float LifeStealPer = SourceSet->GetLifeSteal();
	const float HealPowerPer = SourceSet->GetHealPower();
	if (LifeStealPer <= 0.f) return;

	const float Heal = (Damage * LifeStealPer) * HealPowerPer;
	
	Source->ApplyModToAttribute(
		UPTWAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		Heal
	);
}

void UPTWAbilityBattleAttributeSet::ApplyHealthRegen()
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	const UPTWAttributeSet* PTWSet = ASC->GetSet<UPTWAttributeSet>();
	if (!PTWSet) return;
	
	float CurrentHealth = PTWSet->GetHealth();
	float MaxHealth = PTWSet->GetMaxHealth();
	
	if (CurrentHealth >= MaxHealth)
	{
		StopHealthRegen();
		return;
	}
	
	if (bCanHealthRegen || GetHealthRegen() != 0.f) 
	{
		float HealthRegenAmount = GetHealthRegen() / 10 * GetHealPower();
		
		ASC->ApplyModToAttribute(UPTWAttributeSet::GetHealthAttribute(),EGameplayModOp::Additive, HealthRegenAmount);
	}
}

void UPTWAbilityBattleAttributeSet::ApplyShieldRegen()
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;
	
	const UPTWAttributeSet* PTWSet = ASC->GetSet<UPTWAttributeSet>();
	if (!PTWSet) return;

	float CurrentShield = PTWSet->GetShield();
	
	if (CurrentShield >= GetMaxShield())
	{
		StopShieldRegen();
		return;
	}
	
	if (bCanShieldRegen || GetMaxShield() != 0.f)
	{
		float ShieldRegenAmount = GetShieldRegen() / 10;

		ASC->ApplyModToAttribute(UPTWAttributeSet::GetShieldAttribute(),EGameplayModOp::Additive, ShieldRegenAmount);
	}
}

void UPTWAbilityBattleAttributeSet::StartHealthRegen()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	bCanHealthRegen = true;

	if (World->GetTimerManager().IsTimerActive(HealthRegenLoopTimer)) return;
	
	
	World->GetTimerManager().SetTimer(HealthRegenLoopTimer, this, &UPTWAbilityBattleAttributeSet::ApplyHealthRegen, 0.1f, true);
}

void UPTWAbilityBattleAttributeSet::StopHealthRegen()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(HealthRegenLoopTimer);

	bCanHealthRegen = false;
}

void UPTWAbilityBattleAttributeSet::StartShieldRegen()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	bCanShieldRegen = true;

	if (World->GetTimerManager().IsTimerActive(ShieldRegenLoopTimer)) return;

	World->GetTimerManager().SetTimer(ShieldRegenLoopTimer, this, &UPTWAbilityBattleAttributeSet::ApplyShieldRegen, 0.1f, true);
}

void UPTWAbilityBattleAttributeSet::StopShieldRegen()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(ShieldRegenLoopTimer);
	bCanShieldRegen = false;
}

void UPTWAbilityBattleAttributeSet::ResetShield()
{
	UWorld* World = GetWorld();
	if (!World) return;

	bCanShieldRegen = true;

	World->GetTimerManager().ClearTimer(ShieldRegenTimer);
	World->GetTimerManager().ClearTimer(ShieldRegenLoopTimer);

	StartShieldRegen();
}

void UPTWAbilityBattleAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, Armor, OldArmor);
}

void UPTWAbilityBattleAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, HealthRegen, OldHealthRegen);
}

void UPTWAbilityBattleAttributeSet::OnRep_ShieldRegen(const FGameplayAttributeData& OldShieldRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, ShieldRegen, OldShieldRegen);
}

void UPTWAbilityBattleAttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, LifeSteal, OldLifeSteal);
}

void UPTWAbilityBattleAttributeSet::OnRep_LifeStealOnHit(const FGameplayAttributeData& OldLifeStealOnHit)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, LifeStealOnHit, OldLifeStealOnHit);
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

void UPTWAbilityBattleAttributeSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAbilityBattleAttributeSet, DamageMultiplier, OldDamageMultiplier);
}
