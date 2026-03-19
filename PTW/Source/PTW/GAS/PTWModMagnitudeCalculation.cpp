// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWModMagnitudeCalculation.h"

UPTWModMagnitudeCalculation::UPTWModMagnitudeCalculation()
{
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().WeaponDamageDef);
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().GameADamageMulDef);
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().GameBDamageMulDef);
	
}

float UPTWModMagnitudeCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	
	float BaseDamage = 0.f;
	GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().WeaponDamageDef, Spec, EvaluationParams, BaseDamage);
	
	float FinalMultiplier = 1.0f;
	
	float GameAMul = 0.f;
	if (GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().GameADamageMulDef, Spec, EvaluationParams, GameAMul))
	{
		FinalMultiplier = GameAMul;
	}
	
	else if (float GameBMul = 0.f; GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().GameBDamageMulDef, Spec, EvaluationParams, GameBMul))
	{
		FinalMultiplier = GameBMul;
	}
	
	return FMath::Max(BaseDamage * FinalMultiplier, 0.0f);
}
