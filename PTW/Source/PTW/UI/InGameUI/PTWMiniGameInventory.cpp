// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameInventory.h"
#include "UI/InGameUI/PTWMiniGameItemSlot.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Inventory/Instance/PTWActiveItemInstance.h"
#include "Inventory/Instance/PTWPassiveItemInstance.h"

void UPTWMiniGameInventory::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWMiniGameInventory::InitInventory(UPTWInventoryComponent* InInventory)
{
	InventoryComp = InInventory;

	if (InventoryComp)
	{
		InventoryComp->OnInventoryChanged.AddUObject(this, &ThisClass::RefreshInventory);
	}

	RefreshInventory();
}

void UPTWMiniGameInventory::RefreshInventory()
{
	if (!InventoryComp) return;

	const TArray<TObjectPtr<UPTWItemInstance>>& Items = InventoryComp->GetAllItems();

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
	for (int32 i = 0; i < WeaponSlots.Num(); i++)
	{
		if (!WeaponSlots[i]) continue;

		if (WeaponItems.IsValidIndex(i))
		{
			WeaponSlots[i]->SetItemInstance(WeaponItems[i]);
		}
		else
		{
			WeaponSlots[i]->ClearSlot();
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

	if (UPTWActiveItemInstance* Active = Cast<UPTWActiveItemInstance>(ActiveItem))
	{
		ActiveItemSlot->UpdateCount(Active->GetCurrentCount());
	}
}

void UPTWMiniGameInventory::SetupPassives(const TArray<UPTWItemInstance*>& PassiveItems)
{
	for (int32 i = 0; i < PassiveSlots.Num(); i++)
	{
		if (!PassiveSlots[i]) continue;

		if (PassiveItems.IsValidIndex(i))
		{
			PassiveSlots[i]->SetItemInstance(PassiveItems[i]);
		}
		else
		{
			PassiveSlots[i]->ClearSlot();
		}
	}
}
