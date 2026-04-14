// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBatterLevelWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GAS/PTWDeliveryAttributeSet.h"

void UPTWBatterLevelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsLowbattery = false;
}

void UPTWBatterLevelWidget::NativeDestruct()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute())
			.Remove(BatterLevelChangedHandle);

		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute())
			.Remove(MaxBatterLevelChangedHandle);
	}

	Super::NativeDestruct();
}

void UPTWBatterLevelWidget::BindGASDelegates(UAbilitySystemComponent* ASC)
{
	BatterLevelChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute())
		.AddUObject(this, &ThisClass::OnBatteryLevelAttributeChanged);
	
	MaxBatterLevelChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute())
		.AddUObject(this, &ThisClass::OnMaxBatteryLevelAttributeChanged);
}

void UPTWBatterLevelWidget::InitWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	AbilitySystemComponent = ASC;

	BindGASDelegates(ASC);
	
	float CurBatteryLevel = ASC->GetNumericAttribute(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute());
	float MaxBatteryLevel = ASC->GetNumericAttribute(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute());

	UpdateBatteryLevelBar(CurBatteryLevel, MaxBatteryLevel);
}

void UPTWBatterLevelWidget::OnBatteryLevelAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float MaxBatteryLevel = AbilitySystemComponent
		->GetNumericAttribute(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute());

	UpdateBatteryLevelBar( Data.NewValue, MaxBatteryLevel);
}

void UPTWBatterLevelWidget::OnMaxBatteryLevelAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float CurBatteryLevel = AbilitySystemComponent
		->GetNumericAttribute(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute());

	UpdateBatteryLevelBar(CurBatteryLevel, Data.NewValue);
}

void UPTWBatterLevelWidget::UpdateBatteryLevelBar(float CurrentBatteryLevel, float MaxBatteryLevel)
{
	if (BatteryLevelBar && MaxBatteryLevel > 0.f)
	{
		const float Percent = CurrentBatteryLevel / MaxBatteryLevel;
		BatteryLevelBar->SetPercent(Percent);
		
		bool bCurrentLow = (Percent < 0.2f); 

		if (bIsLowbattery != bCurrentLow)
		{
			UpdateBatteryUI(bCurrentLow);
		}
	}
}

void UPTWBatterLevelWidget::UpdateBatteryUI(bool bIsLow)
{
	if (!BatteryLevelBar) return;
	
	bIsLowbattery = bIsLow;
	
	FProgressBarStyle BatteryStyle = BatteryLevelBar->GetWidgetStyle();
	FLinearColor NextColor = bIsLow ? FLinearColor::Red : FLinearColor::Green;
	
	int32 ImageIndex = bIsLow ? 0 : 1;
	if (BatterFillImage.IsValidIndex(ImageIndex))
	{
		UTexture2D* NewTexture = BatterFillImage[ImageIndex];
		if (NewTexture)
		{
			BatteryStyle.FillImage.SetResourceObject(NewTexture);
			BatteryLevelBar->SetWidgetStyle(BatteryStyle);
		}
	}
	BatteryLevelBar->SetFillColorAndOpacity(NextColor);
}
