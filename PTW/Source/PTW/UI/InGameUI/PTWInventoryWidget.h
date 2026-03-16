// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/PTWPlayerData.h"
#include "PTWInventoryWidget.generated.h"

class UUniformGridPanel;
class UPTWItemSlot;
class APTWPlayerState;
class UDataTable;
class UPTWItemDefinition;

/**
 * 
 */
UCLASS()
class PTW_API UPTWInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitPS();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 델리게이트 연결 */
	void BindDelegates();
	void UnbindDelegates();

	/* 델리게이트에 바인딩될 함수 */
	UFUNCTION()
	void OnInventoryUpdated(const FPTWPlayerData& NewData);

	void CreateSlot(UUniformGridPanel* TargetGrid, UPTWItemDefinition* ItemDef, int32 Index);

	/* 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> WeaponGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ActiveItemGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> PassiveItemGrid;

	/* 에디터에서 PTWItemSlot 블루프린트 클래스를 할당하기 위한 변수 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UPTWItemSlot> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxRowsPerColumn = 10;

private:
	TWeakObjectPtr<APTWPlayerState> CachedPlayerState;
};
