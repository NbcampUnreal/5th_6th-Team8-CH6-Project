// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWItemInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWItemInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetItemID(FName InItemID);

protected:
	void Refresh();

	/* 이름 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_ItemName;
	/* 가격 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Price;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Text_PriceDelta;
	/* 설명 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Description;

	UPROPERTY()
	FName ItemID;
};
