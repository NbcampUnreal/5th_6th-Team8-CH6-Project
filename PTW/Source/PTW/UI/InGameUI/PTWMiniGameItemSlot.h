// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PTWMiniGameItemSlot.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UAbilitySystemComponent; 
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

	void InitCooldown(UAbilitySystemComponent* ASC, const FGameplayTag& InCooldownTag);

	void UpdateCount(int32 NewCount);

	void ClearSlot();

	void ResetCooldownUI();

protected:

	virtual void NativeDestruct() override;

private:

	void OnCooldownTagChanged(const FGameplayTag Tag, int32 NewCount);

	void UpdateCooldownDisplay();

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> CooldownProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

private:

	UPROPERTY()
	TObjectPtr<UPTWItemInstance> ItemInstance;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FGameplayTag CooldownTag;

	FDelegateHandle CooldownTagDelegateHandle;

	FTimerHandle CooldownUpdateTimer;
};
