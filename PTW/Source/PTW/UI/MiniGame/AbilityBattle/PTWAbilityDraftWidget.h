// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "PTWAbilityDraftWidget.generated.h"

class UPTWShieldBar;
class UPTWDraftCharge;
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

	void OnShieldChanged(const FOnAttributeChangeData& Data);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWDraftCharge> WBP_DraftCharge;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWShieldBar> WBP_ShieldBar;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UPTWAbilityBoxWidget> AbilityBoxClass;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<UDataTable> AbilityDraftDataTable;

	bool bIsSelected = false;

protected:
	virtual void NativeConstruct() override;
private:

	
};
