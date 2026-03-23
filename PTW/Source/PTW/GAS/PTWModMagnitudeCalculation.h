// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "PTWAbilityBattleAttributeSet.h"
#include "PTWWeaponAttributeSet.h"
#include "PTWModMagnitudeCalculation.generated.h"

struct FPTWDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition WeaponDamageDef;
	FGameplayEffectAttributeCaptureDefinition DefenseDef;
	
	FPTWDamageStatics()
	{
		WeaponDamageDef = FGameplayEffectAttributeCaptureDefinition(
			UPTWWeaponAttributeSet::GetDamageAttribute(), 
			EGameplayEffectAttributeCaptureSource::Source, 
			false);
		
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(
			UPTWAttributeSet::GetDefenseAttribute(),
			EGameplayEffectAttributeCaptureSource::Target, 
			false);
	}
	
	static const FPTWDamageStatics& DamageStatics() { static FPTWDamageStatics DStatics; return DStatics; }
};

/**
 * 
 */
UCLASS()
class PTW_API UPTWModMagnitudeCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UPTWModMagnitudeCalculation();
	
	float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
