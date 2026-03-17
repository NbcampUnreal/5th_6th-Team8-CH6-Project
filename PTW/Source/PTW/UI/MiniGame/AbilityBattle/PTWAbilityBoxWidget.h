// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWAbilityBoxWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDraftSelected, FName);

class UButton;
class UTextBlock;
class UImage;

UCLASS()
class PTW_API UPTWAbilityBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitAbilityBoxWidget(FName RowId, UDataTable* DataTable);

	UFUNCTION()
	void OnButtonClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_AbilityButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_AbilityIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AbilityName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AbilityDescription;
	
	FOnDraftSelected OnDraftSelected;
private:
	FName CachedRowId;
	
};
