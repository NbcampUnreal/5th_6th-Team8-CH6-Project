// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PTWWeaponAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PTW_API UPTWWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPTWWeaponAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxAmmo, Category = "Weapon|Attributes")
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UPTWWeaponAttributeSet, MaxAmmo);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAmmo, Category = "Weapon|Attributes")
	FGameplayAttributeData CurrentAmmo;
	ATTRIBUTE_ACCESSORS(UPTWWeaponAttributeSet, CurrentAmmo);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Damage, Category = "Weapon|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UPTWWeaponAttributeSet, Damage);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireRate, Category = "Weapon|Attributes")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UPTWWeaponAttributeSet, FireRate);
	
protected:
	UFUNCTION()
	void OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo);
	
	UFUNCTION()
	void OnRep_Damage(const FGameplayAttributeData& OldDamage);
	
	UFUNCTION()
	void OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo);
	
	UFUNCTION()
	void OnRep_FireRate(const FGameplayAttributeData& OldFireRate);
};
