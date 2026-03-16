// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameItemSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/PTWItemDefinition.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

void UPTWMiniGameItemSlot::SetItemInstance(UPTWItemInstance* InItem)
{
	ItemInstance = InItem;

	if (!ItemInstance)
	{
		ClearSlot();
		return;
	}

	UPTWItemDefinition* ItemDef = ItemInstance->GetItemDef();
	if (!ItemDef) return;

	/* 아이콘 설정 */
	if (ItemIcon && ItemDef->ItemIcon.IsValid())
	{
		UTexture2D* IconTex = ItemDef->ItemIcon.Get();
		ItemIcon->SetBrushFromTexture(IconTex);
	}

	/* 쿨타임 머터리얼 생성 */
	if (CooldownImage)
	{
		CooldownMID = CooldownImage->GetDynamicMaterial();
	}

	/* 기본적으로 Count 숨김 */
	if (CountText)
	{
		CountText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWMiniGameItemSlot::UpdateCount(int32 NewCount)
{
	if (!CountText) return;

	if (NewCount <= 0)
	{
		CountText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CountText->SetVisibility(ESlateVisibility::Visible);
	CountText->SetText(FText::AsNumber(NewCount));
}

void UPTWMiniGameItemSlot::UpdateCooldown(float RemainingTime, float TotalTime)
{
	CooldownRemaining = RemainingTime;
	CooldownDuration = TotalTime;
}

void UPTWMiniGameItemSlot::ClearSlot()
{
	ItemInstance = nullptr;

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(nullptr);
	}

	if (CountText)
	{
		CountText->SetVisibility(ESlateVisibility::Collapsed);
	}

	CooldownRemaining = 0.f;
	CooldownDuration = 0.f;
}

void UPTWMiniGameItemSlot::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CooldownMID) return;
	if (CooldownDuration <= 0.f) return;

	CooldownRemaining -= InDeltaTime;

	float Ratio = FMath::Clamp(CooldownRemaining / CooldownDuration, 0.f, 1.f);

	CooldownMID->SetScalarParameterValue(TEXT("Percent"), Ratio);
}
