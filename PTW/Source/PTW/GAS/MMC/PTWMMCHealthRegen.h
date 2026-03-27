// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "PTWMMCHealthRegen.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWMMCHealthRegen : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UPTWMMCHealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

};
