// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWHealthBar.h"
#include "Components/ProgressBar.h" 
#include "AbilitySystemComponent.h" // ASC
#include "GAS/PTWAttributeSet.h"

void UPTWHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWHealthBar::NativeDestruct()
{
	/* 델리게이트 Unbind (UI 파괴될 때 크래시 방지) */
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);

		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetMaxHealthAttribute())
			.Remove(MaxHealthChangedHandle);
	}

	Super::NativeDestruct();
}

void UPTWHealthBar::InitWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	AbilitySystemComponent = ASC;

	BindGASDelegates(ASC);

	/* 위젯 초기 표시값 설정 */
	float CurrentHealth = ASC->GetNumericAttribute(UPTWAttributeSet::GetHealthAttribute());
	float MaxHealth = ASC->GetNumericAttribute(UPTWAttributeSet::GetMaxHealthAttribute());

	UpdateHealthBar(CurrentHealth, MaxHealth);
}

void UPTWHealthBar::BindGASDelegates(UAbilitySystemComponent* ASC)
{
	/* Health 변경 감지 */
	HealthChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UPTWHealthBar::OnHealthAttributeChanged);

	/* MaxHealth 변경 감지 */
	MaxHealthChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UPTWHealthBar::OnMaxHealthAttributeChanged);
}

void UPTWHealthBar::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float MaxHealth = AbilitySystemComponent
		->GetNumericAttribute(UPTWAttributeSet::GetMaxHealthAttribute());

	UpdateHealthBar(Data.NewValue, MaxHealth);
}

void UPTWHealthBar::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float CurrentHealth = AbilitySystemComponent
		->GetNumericAttribute(UPTWAttributeSet::GetHealthAttribute());

	UpdateHealthBar(CurrentHealth, Data.NewValue);
}

void UPTWHealthBar::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	if (HealthBar && MaxHealth > 0.f)
	{
		const float Percent = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(Percent);
	}
}
