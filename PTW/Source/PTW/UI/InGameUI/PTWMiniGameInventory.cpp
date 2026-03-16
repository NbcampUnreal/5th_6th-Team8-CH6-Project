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

void UPTWMiniGameInventory::NativeConstruct()
{
	Super::NativeConstruct();

	AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary
		::GetAbilitySystemComponent(GetOwningPlayerPawn());
}

void UPTWMiniGameInventory::NativeDestruct()
{
	if (InventoryComp)
	{
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
	InventoryComp = InInventory;

	if (InventoryComp)
	{
		InventoryComp->OnInventoryChanged
			.AddUObject(this, &ThisClass::RefreshInventory);
	}

	RefreshInventory();
}

void UPTWMiniGameInventory::RefreshInventory()
{
	if (!InventoryComp) return;

	const TArray<TObjectPtr<UPTWItemInstance>>& Items =
		InventoryComp->GetAllItems();

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
		if (!Slot) continue;

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

	if (!ActiveItem)
	{
		ActiveItemSlot->ClearSlot();
		return;
	}

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
