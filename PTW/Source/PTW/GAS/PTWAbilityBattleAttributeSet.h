// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWAttributeSet.h"
#include "PTWAbilityBattleAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class PTW_API UPTWAbilityBattleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UPTWAbilityBattleAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleDamaged(UAbilitySystemComponent* Target, UAbilitySystemComponent* Source, float Damage);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, Armor);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegen, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, HealthRegen );

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShieldRegen, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData ShieldRegen;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, ShieldRegen);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LifeSteal, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData LifeSteal;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, LifeSteal );

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, Shield);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LifeStealOnHit, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData LifeStealOnHit;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, LifeStealOnHit);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, MaxShield);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageReceived, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData DamageReceived;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, DamageReceived);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReflectDamagePercent , Category = "AttributeBattle|Attributes")
	FGameplayAttributeData ReflectDamagePercent ;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, ReflectDamagePercent );
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealPower, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData HealPower;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, HealPower);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageMultiplier, Category = "AttributeBattle|Attributes")
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS(UPTWAbilityBattleAttributeSet, DamageMultiplier);

protected:
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen);

	UFUNCTION()
	void OnRep_ShieldRegen(const FGameplayAttributeData& OldShieldRegen);
	
	UFUNCTION()
	void OnRep_LifeSteal (const FGameplayAttributeData& OldLifeSteal);

	UFUNCTION()
	void OnRep_LifeStealOnHit(const FGameplayAttributeData& OldLifeStealOnHit);

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

	UFUNCTION()
	void OnRep_DamageReceived(const FGameplayAttributeData& OldDamageReceived);

	UFUNCTION()
	void OnRep_ReflectDamagePercent(const FGameplayAttributeData& OldReflectDamagePercent);

	UFUNCTION()
	void OnRep_HealPower(const FGameplayAttributeData& OldHealPower);

	UFUNCTION()
	void OnRep_DamageMultiplier (const FGameplayAttributeData& OldDamageMultiplier);
	
public:
	void ApplyHealthRegen();
	void ApplyShieldRegen();
	
	void StartHealthRegen();
	void StopHealthRegen();

	void StartShieldRegen();
	void StopShieldRegen();
	
	float HealthRegenDelay = 10.f;
	float ShieldRegenDelay = 5.f;
	
	bool bCanHealthRegen = false;
	bool bCanShieldRegen = false;

	FTimerHandle HealthRegenTimer;
	FTimerHandle ShieldRegenTimer;

	FTimerHandle HealthRegenLoopTimer;
	FTimerHandle ShieldRegenLoopTimer;
};
