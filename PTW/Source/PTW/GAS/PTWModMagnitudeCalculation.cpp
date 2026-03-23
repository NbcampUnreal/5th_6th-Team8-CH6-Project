// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWModMagnitudeCalculation.h"

UPTWModMagnitudeCalculation::UPTWModMagnitudeCalculation()
{
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().WeaponDamageDef);
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().DefenseDef);
	
}

float UPTWModMagnitudeCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;
	
	float RawDamage = 0.0f;
	GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().WeaponDamageDef, Spec, EvaluationParams, RawDamage);

	float DefensePoint = 0.0f;
	GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().DefenseDef, Spec, EvaluationParams, DefensePoint);
	
	DefensePoint = FMath::Max<float>(DefensePoint, 0.0f);
	const float DamageReduction = DefensePoint / (DefensePoint + 100.0f);
	
	float FinalDamage = RawDamage * (1.0f - DamageReduction);
	
	return FMath::Max<float>(FinalDamage, 1.0f);
}
