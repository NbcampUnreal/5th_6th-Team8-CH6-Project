// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "PTWAbilityBattleAttributeSet.h"
#include "PTWWeaponAttributeSet.h"
#include "PTWModMagnitudeCalculation.generated.h"

struct FPTWDamageStatics
{
	// 공통 무기 데미지
	FGameplayEffectAttributeCaptureDefinition WeaponDamageDef;
    
	// 미니게임 A용 배율
	FGameplayEffectAttributeCaptureDefinition GameADamageMulDef; //
	// 미니게임 B용 배율
	FGameplayEffectAttributeCaptureDefinition GameBDamageMulDef;
	
	// 방어력 캡쳐

	FPTWDamageStatics()
	{
		// 1. 공통 무기 데미지 캐싱
		WeaponDamageDef = FGameplayEffectAttributeCaptureDefinition(UPTWWeaponAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, false);
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
