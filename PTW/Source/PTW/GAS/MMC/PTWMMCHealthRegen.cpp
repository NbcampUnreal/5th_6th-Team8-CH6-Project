// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC/PTWMMCHealthRegen.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"


struct FHealthRegenStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealthRegen);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealPower);

	FHealthRegenStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPTWAbilityBattleAttributeSet, HealthRegen, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPTWAbilityBattleAttributeSet, HealPower, Source, false);
	}
};

static FHealthRegenStatics& GetStatics()
{
	static FHealthRegenStatics Statics;
	return Statics;
}

UPTWMMCHealthRegen::UPTWMMCHealthRegen()
{
	RelevantAttributesToCapture.Add(GetStatics().HealthRegenDef);
	RelevantAttributesToCapture.Add(GetStatics().HealPowerDef);
}

float UPTWMMCHealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters Params;

	float HealthRegen = 0.f;
	float HealPower = 0.f;

	GetCapturedAttributeMagnitude(GetStatics().HealthRegenDef, Spec, Params, HealthRegen);
	GetCapturedAttributeMagnitude(GetStatics().HealPowerDef, Spec, Params, HealPower);
	
	HealthRegen = FMath::Max(HealthRegen, 0.f);
	HealPower = FMath::Max(HealPower, 0.f);

	const float Result = HealthRegen/ 10 * HealPower;

	return Result;

	
}
