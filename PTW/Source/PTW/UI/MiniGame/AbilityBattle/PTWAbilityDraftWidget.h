// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWAbilityDraftWidget.generated.h"

class UPTWAbilityBoxWidget;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class PTW_API UPTWAbilityDraftWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void GenerateAbilityBoxes(TArray<FName> RowId);

	UFUNCTION()
	void OnDraftSelected(FName RowId);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UPTWAbilityBoxWidget> AbilityBoxClass;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<UDataTable> AbilityDraftDataTable;

	bool bIsSelected = false;
private:
	
};
