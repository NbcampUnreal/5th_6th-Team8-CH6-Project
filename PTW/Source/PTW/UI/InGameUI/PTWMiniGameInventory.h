// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMiniGameInventory.generated.h"

class UPTWMiniGameItemSlot;
class UPTWInventoryComponent;
class UPTWItemInstance;

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameInventory : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void InitInventory(UPTWInventoryComponent* InInventory);

	void RefreshInventory();

protected:

	virtual void NativeConstruct() override;

protected:

	UPROPERTY(meta = (BindWidget))
	TArray<TObjectPtr<UPTWMiniGameItemSlot>> WeaponSlots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameItemSlot> ActiveItemSlot;

	UPROPERTY(meta = (BindWidget))
	TArray<TObjectPtr<UPTWMiniGameItemSlot>> PassiveSlots;

private:

	UPROPERTY()
	TObjectPtr<UPTWInventoryComponent> InventoryComp;

	void SetupWeapons(const TArray<UPTWItemInstance*>& WeaponItems);
	void SetupActive(UPTWItemInstance* ActiveItem);
	void SetupPassives(const TArray<UPTWItemInstance*>& PassiveItems);
};
