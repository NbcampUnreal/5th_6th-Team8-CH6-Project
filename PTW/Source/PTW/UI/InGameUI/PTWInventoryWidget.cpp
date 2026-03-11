// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWInventoryWidget.h"
#include "UI/InGameUI/PTWItemSlot.h"
#include "Components/UniformGridPanel.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Inventory/PTWItemDefinition.h"
#include "Gameplay/Shop/PTWShopItemData.h" 
#include "Engine/DataTable.h"
#include "System/Shop/PTWShopSubsystem.h"

void UPTWInventoryWidget::InitPS()
{
	if (APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>())
	{
		CachedPlayerState = PS;
	}

	BindDelegates();
}

void UPTWInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWInventoryWidget::NativeDestruct()
{
	UnbindDelegates();

	Super::NativeDestruct();
}

void UPTWInventoryWidget::BindDelegates()
{
	/* 중복 바인딩 방지 */
	UnbindDelegates();

	/* 델리게이트 바인딩 */
	if (CachedPlayerState.IsValid())
	{
		CachedPlayerState->OnPlayerDataUpdated.AddDynamic(this, &UPTWInventoryWidget::OnInventoryUpdated);

		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget Binding delegates"));
		
		/* 최신 데이터로 초기화 */
		OnInventoryUpdated(CachedPlayerState->GetPlayerData());
	}

	
}

void UPTWInventoryWidget::UnbindDelegates()
{
	/* 델리게이트 바인딩 해제 */
	if (CachedPlayerState.IsValid())
	{
		CachedPlayerState->OnPlayerDataUpdated.RemoveDynamic(this, &UPTWInventoryWidget::OnInventoryUpdated);
	}
}

void UPTWInventoryWidget::OnInventoryUpdated(const FPTWPlayerData& NewData)
{
	if (!InventoryGrid || !SlotWidgetClass || !ItemDataTable)
		return;

	InventoryGrid->ClearChildren();

	for (int32 i = 0; i < NewData.InventoryItemIDs.Num(); ++i)
	{
		const FString& ItemID = NewData.InventoryItemIDs[i];

		// DataTable에서 행 찾기
		FName RowName(*ItemID);
		FShopItemRow* Row = ItemDataTable->FindRow<FShopItemRow>(
			RowName,
			TEXT("Inventory Lookup")
		);

		if (!Row)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemID [%s] not found in DataTable"), *ItemID);
			continue;
		}

		// ItemDefinition 꺼내기
		UPTWItemDefinition* ItemDef = Row->ItemDefinition.LoadSynchronous();

		if (!ItemDef)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemID [%s] has NULL ItemDefinition"), *ItemID);
			continue;
		}

		UPTWItemSlot* NewSlot = CreateWidget<UPTWItemSlot>(this, SlotWidgetClass);
		if (!NewSlot)
			continue;

		NewSlot->SetupSlot(ItemDef);

		int32 RowIdx = i / MaxColumns;
		int32 ColIdx = i % MaxColumns;

		InventoryGrid->AddChildToUniformGrid(NewSlot, RowIdx, ColIdx);
	}
}
