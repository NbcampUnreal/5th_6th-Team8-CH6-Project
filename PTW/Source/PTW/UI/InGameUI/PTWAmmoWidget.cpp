// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWAmmoWidget.h"
#include "Components/TextBlock.h"

void UPTWAmmoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기값 (선택)
	UpdateAmmoWidget(0, 0);
}

void UPTWAmmoWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPTWAmmoWidget::UpdateAmmoWidget(int32 CurrentAmmo, int32 ReserveAmmo)
{
	if (CurrentAmmoText)
		CurrentAmmoText->SetText(FText::AsNumber(CurrentAmmo));

	if (ReserveAmmoText)
		ReserveAmmoText->SetText(FText::AsNumber(ReserveAmmo));
}
