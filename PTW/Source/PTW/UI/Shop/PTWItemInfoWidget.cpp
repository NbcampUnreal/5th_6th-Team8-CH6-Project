// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PTWItemInfoWidget.h"
#include "Components/TextBlock.h"
#include "System/Shop/PTWShopSubsystem.h"

void UPTWItemInfoWidget::SetItemID(FName InItemID)
{
	ItemID = InItemID;
	Refresh();
}

void UPTWItemInfoWidget::Refresh()
{
	if (ItemID.IsNone()) return;

	if (UPTWShopSubsystem* ShopSys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		const FShopItemRow* Data = ShopSys->GetShopItemData(ItemID);
		if (!Data) return;

		/* 아이템 이름 */
		if (Text_ItemName)
		{
			Text_ItemName->SetText(Data->DisplayName);
		}

		/* 아이템 가격 */
		const int32 BasePrice = Data->BasePrice;
		const int32 FinalPrice = ShopSys->GetItemPrice(ItemID);

		if (Text_Price)
		{
			Text_Price->SetText(FText::Format(NSLOCTEXT("Shop", "Price", "{0} G"), FinalPrice));
		}

		if (Text_PriceDelta)
		{
			if (FinalPrice > BasePrice)
			{
				Text_PriceDelta->SetText(FText::FromString(TEXT("⬆")));
				Text_PriceDelta->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
				Text_PriceDelta->SetVisibility(ESlateVisibility::Visible);
			}
			else if (FinalPrice < BasePrice)
			{
				Text_PriceDelta->SetText(FText::FromString(TEXT("⬇")));
				Text_PriceDelta->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
				Text_PriceDelta->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				Text_PriceDelta->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		/* 아이템 설명 */
		if (Text_Description)
		{
			Text_Description->SetText(Data->Description);
		}
	}
}
