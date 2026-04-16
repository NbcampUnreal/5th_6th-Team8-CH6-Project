// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameInventory.h"
#include "UI/InGameUI/PTWMiniGameItemSlot.h"

#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Inventory/Instance/PTWActiveItemInstance.h"
#include "Inventory/Instance/PTWPassiveItemInstance.h"
#include "Inventory/PTWItemDefinition.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

/* 로그용 */
#include "CoreFramework/PTWPlayerState.h"

void UPTWMiniGameInventory::NativeConstruct()
{
	Super::NativeConstruct();

	AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary
		::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (ActiveItemSlot)
	{
		ActiveItemSlot->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTWMiniGameInventory::NativeDestruct()
{
	if (InventoryComp)
	{
		InventoryComp->OnInventoryChanged.RemoveAll(this);

		for (auto Item : InventoryComp->GetAllItems())
		{
			if (UPTWActiveItemInstance* Active = Cast<UPTWActiveItemInstance>(Item))
			{
				Active->OnItemDepleted.RemoveAll(this);
			}
		}
	}
	Super::NativeDestruct();
}

void UPTWMiniGameInventory::InitInventory(UPTWInventoryComponent* InInventory)
{
	if (InventoryComp)
	{
		InventoryComp->OnInventoryChanged.RemoveAll(this);
	}

	InventoryComp = InInventory;

	if (InventoryComp)
	{
		InventoryComp->OnInventoryChanged
			.AddUObject(this, &ThisClass::RefreshInventory);
	}

	RefreshInventory();

	/* 로그용 */
	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();

	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameInventory] %s 플레이어 InitInventory 완료."),
		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
}

void UPTWMiniGameInventory::RefreshInventory()
{
	/* 로그용 */
	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
	FString PlayerName = PS ? *PS->GetPlayerName() : TEXT("Unknown");

	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameInventory] %s 플레이어 RefreshInventory 호출됨."),
		PS ? *PS->GetPlayerName() : TEXT("Unknown"));

	if (!InventoryComp) return;

	const TArray<TObjectPtr<UPTWItemInstance>>& Items =
		InventoryComp->GetAllItems();

	// --- 전체 아이템 리스트 출력 시작 ---
	UE_LOG(LogTemp, Error, TEXT("[MiniGameInventory]========= [%s] 인벤토리 상세 목록 (총 %d개) ========="), *PlayerName, Items.Num());

	for (int32 i = 0; i < Items.Num(); i++)
	{
		UPTWItemInstance* Item = Items[i];
		if (!Item) continue;

		UPTWItemDefinition* Def = Item->GetItemDef();

		// DisplayName 추출 (FText -> FString)
		FString NameToPrint = Def ? Def->DisplayName.ToString() : TEXT("이름 없음");

		// 아이템 타입 (Enum) 추출
		FString TypeName = TEXT("알 수 없음");
		if (Def)
		{
			switch (Def->ItemType)
			{
			case EItemType::Weapon:  TypeName = TEXT("무기"); break;
			case EItemType::Active:  TypeName = TEXT("액티브"); break;
			case EItemType::Passive: TypeName = TEXT("패시브"); break;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[MiniGameInventory][%d] 표시 이름: %s | 타입: %s"), i, *NameToPrint, *TypeName);
	}
	// --- 전체 아이템 리스트 출력 종료 ---

	TArray<UPTWItemInstance*> WeaponItems;
	TArray<UPTWItemInstance*> PassiveItems;
	UPTWItemInstance* ActiveItem = nullptr;

	for (UPTWItemInstance* Item : Items)
	{
		if (!Item) continue;

		if (Item->IsA(UPTWWeaponInstance::StaticClass()))
		{
			WeaponItems.Add(Item);
		}
		else if (Item->IsA(UPTWActiveItemInstance::StaticClass()))
		{
			ActiveItem = Item;
		}
		else if (Item->IsA(UPTWPassiveItemInstance::StaticClass()))
		{
			PassiveItems.Add(Item);
		}
	}

	// --- WeaponItems 분류 결과 상세 확인 로그 ---
	UE_LOG(LogTemp, Error, TEXT("[MiniGameInventory]---------- [WeaponItems 분류 결과] ----------"));
	UE_LOG(LogTemp, Warning, TEXT("무기 슬롯에 들어갈 아이템 수: %d"), WeaponItems.Num());

	for (int32 j = 0; j < WeaponItems.Num(); j++)
	{
		UPTWItemInstance* WepInst = WeaponItems[j];
		if (WepInst)
		{
			UPTWItemDefinition* Def = WepInst->GetItemDef();
			FString WepName = Def ? Def->DisplayName.ToString() : TEXT("이름 없는 무기");

			// 메모리 주소(Ptr)를 같이 찍어보면 실제로 같은 객체인지, 
			// 아니면 이름만 같은 별개의 객체가 생성된 건지 확실히 알 수 있습니다.
			UE_LOG(LogTemp, Log, TEXT("[MiniGameInventory]무기 [%d]: %s (주소: %p)"), j, *WepName, WepInst);
		}
	}
	UE_LOG(LogTemp, Error, TEXT("[MiniGameInventory]======================================================"));

	SetupWeapons(WeaponItems);
	SetupActive(ActiveItem);
	SetupPassives(PassiveItems);
}

void UPTWMiniGameInventory::SetupWeapons(const TArray<UPTWItemInstance*>& WeaponItems)
{
	if (!WeaponGrid) return;

	WeaponGrid->ClearChildren();

	for (int32 i = 0; i < WeaponItems.Num(); i++)
	{
		UPTWItemInstance* Item = WeaponItems[i];
		if (!Item) continue;

		UPTWMiniGameItemSlot* ItemSlot = CreateSlot();
		if (!ItemSlot) continue;

		UUniformGridSlot* GridSlot = WeaponGrid->AddChildToUniformGrid(ItemSlot);

		GridSlot->SetRow(0);
		GridSlot->SetColumn(i);

		ItemSlot->SetItemInstance(Item);

		if (AbilitySystemComponent)
		{
			if (UPTWItemDefinition* Def = Item->GetItemDef())
			{
				if (Def->CooldownTag.IsValid())
				{
					ItemSlot->InitCooldown(
						AbilitySystemComponent,
						Def->CooldownTag);
				}
			}
		}
	}
}

void UPTWMiniGameInventory::SetupActive(UPTWItemInstance* ActiveItem)
{
	if (!ActiveItemSlot) return;

	/* 로그용 */
	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();

	if (!IsValid(ActiveItem))
	{
		ActiveItemSlot->ClearSlot();
		ActiveItemSlot->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	ActiveItemSlot->SetVisibility(ESlateVisibility::Visible);

	ActiveItemSlot->SetItemInstance(ActiveItem);

	if (UPTWActiveItemInstance* ActiveInstance = Cast<UPTWActiveItemInstance>(ActiveItem))
	{
		// 중복 바인딩 방지
		ActiveInstance->OnItemDepleted.RemoveAll(this);

		// 델리게이트 바인딩
		ActiveInstance->OnItemDepleted.AddUObject(this, &UPTWMiniGameInventory::EraseActive);
	}

	if (AbilitySystemComponent)
	{
		if (UPTWItemDefinition* Def = ActiveItem->GetItemDef())
		{
			if (Def->CooldownTag.IsValid())
			{
				ActiveItemSlot->InitCooldown(
					AbilitySystemComponent,
					Def->CooldownTag);
			}
			else
			{
				ActiveItemSlot->ResetCooldownUI();
			}
		}
	}

	if (UPTWActiveItemInstance* Active =
		Cast<UPTWActiveItemInstance>(ActiveItem))
	{
		ActiveItemSlot->UpdateCount(Active->GetCurrentCount());
	}
}

void UPTWMiniGameInventory::EraseActive()
{
	if (!ActiveItemSlot) return;

	ActiveItemSlot->ClearSlot();
	ActiveItemSlot->SetVisibility(ESlateVisibility::Hidden);
}

void UPTWMiniGameInventory::SetupPassives(
	const TArray<UPTWItemInstance*>& PassiveItems)
{
	if (!PassiveGrid) return;

	PassiveGrid->ClearChildren();

	for (int32 i = 0; i < PassiveItems.Num(); i++)
	{
		UPTWItemInstance* Item = PassiveItems[i];
		if (!Item) continue;

		UPTWMiniGameItemSlot* ItemSlot = CreateSlot();
		if (!ItemSlot) continue;

		int32 Row = i / 4;
		int32 Col = i % 4;

		UUniformGridSlot* GridSlot = PassiveGrid->AddChildToUniformGrid(ItemSlot);

		GridSlot->SetRow(Row);
		GridSlot->SetColumn(Col);

		ItemSlot->SetItemInstance(Item);
	}
}

UPTWMiniGameItemSlot* UPTWMiniGameInventory::CreateSlot()
{
	if (!ItemSlotClass) return nullptr;

	return CreateWidget<UPTWMiniGameItemSlot>(
		GetOwningPlayer(),
		ItemSlotClass);
}
