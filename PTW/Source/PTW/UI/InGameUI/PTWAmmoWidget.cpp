// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWAmmoWidget.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h" // ASC
#include "GAS/PTWWeaponAttributeSet.h" 
#include "GameplayTagContainer.h"


void UPTWAmmoWidget::InitWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	AbilitySystemComponent = ASC;

	BindGASDelegates(ASC);

	/* 위젯 초기 표시값 설정 */
	float CurrentAmmo = ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute());
	float MaxAmmo = ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetMaxAmmoAttribute());

	UpdateAmmoWidget(CurrentAmmo, MaxAmmo);
	SetVisibility(ESlateVisibility::Hidden);
}

void UPTWAmmoWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWAmmoWidget::NativeDestruct()
{
	/* 델리게이트 Unbind (UI 파괴될 때 크래시 방지) */
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute())
			.Remove(CurrentAmmoChangedHandle);

		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetMaxAmmoAttribute())
			.Remove(MaxAmmoChangedHandle);

		if (EquipWeaponHandle.IsValid())
		{
			AbilitySystemComponent
				->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("Weapon.State.Equip")), EGameplayTagEventType::NewOrRemoved)
				.Remove(EquipWeaponHandle);

			EquipWeaponHandle.Reset();
		}
	}

	Super::NativeDestruct();
}

void UPTWAmmoWidget::BindGASDelegates(UAbilitySystemComponent* ASC)
{
	/* CurrentAmmo 변경 감지 */
	CurrentAmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute())
		.AddUObject(this, &UPTWAmmoWidget::OnCurrentAmmoAttributeChanged);

	/* MaxAmmo 변경 감지 */
	MaxAmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWWeaponAttributeSet::GetMaxAmmoAttribute())
		.AddUObject(this, &UPTWAmmoWidget::OnMaxAmmoAttributeChanged);

	/* 무기 장착 여부 감지 */
	EquipWeaponHandle =
		ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")), EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWAmmoWidget::SetAmmoWidgetVisibility);
}

void UPTWAmmoWidget::OnCurrentAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float MaxAmmo = AbilitySystemComponent
		->GetNumericAttribute(UPTWWeaponAttributeSet::GetMaxAmmoAttribute());

	UpdateAmmoWidget(Data.NewValue, MaxAmmo);
}


void UPTWAmmoWidget::OnMaxAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent) return;

	float CurrentAmmo = AbilitySystemComponent
		->GetNumericAttribute(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute());

	UpdateAmmoWidget(CurrentAmmo, Data.NewValue);
}

void UPTWAmmoWidget::UpdateAmmoWidget(float CurrentAmmo, float MaxAmmo)
{
	if (CurrentAmmoText)
		CurrentAmmoText->SetText(FText::AsNumber(FMath::RoundToInt(CurrentAmmo)));

	if (MaxAmmoText)
		MaxAmmoText->SetText(FText::AsNumber(FMath::RoundToInt(MaxAmmo)));
}

void UPTWAmmoWidget::SetAmmoWidgetVisibility(const FGameplayTag Tag, int32 NewCount)
{
	const bool bEquipped = (NewCount > 0);
	SetVisibility(bEquipped ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
