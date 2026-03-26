// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "PTWShieldBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class PTW_API UPTWShieldBar : public UUserWidget
{
	GENERATED_BODY()


public:
	void SetProgressBarPer(float Current, float Max);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Shield;
	
};
