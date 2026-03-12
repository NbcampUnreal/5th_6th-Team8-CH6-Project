// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "PTWBatterLevelWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PTW_API UPTWBatterLevelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void InitWithASC(UAbilitySystemComponent* ASC);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> BatteryLevelBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BatteryLevelTextBlock;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	void BindGASDelegates(UAbilitySystemComponent* ASC);
	
	void OnBatteryLevelAttributeChanged(const FOnAttributeChangeData& Data);

	void OnMaxBatteryLevelAttributeChanged(const FOnAttributeChangeData& Data);

	void UpdateBatteryLevelBar(float CurrentBatteryLevel, float MaxBatteryLevel);
	
	void UpdateBatteryText(float Percent);
	
	FDelegateHandle BatterLevelChangedHandle;
	FDelegateHandle MaxBatterLevelChangedHandle;
};
