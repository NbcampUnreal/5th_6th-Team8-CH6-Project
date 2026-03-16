// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMiniGameItemSlot.generated.h"

class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;
class UPTWItemInstance;

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameItemSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetItemInstance(UPTWItemInstance* InItem);

	void UpdateCount(int32 NewCount);

	void UpdateCooldown(float RemainingTime, float TotalTime);

	void ClearSlot();

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CooldownImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

private:

	UPROPERTY()
	TObjectPtr<UPTWItemInstance> ItemInstance;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMID;

	float CooldownRemaining = 0.f;
	float CooldownDuration = 0.f;
};
