// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWPredictPlayerEntry.generated.h"

DECLARE_DELEGATE_OneParam(FOnPlayerEntryClicked, const FString&);
/**
 * 
 */
UCLASS()
class PTW_API UPTWPredictPlayerEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntryData(const FString& InPlayerName);
	FOnPlayerEntryClicked OnClickedDelegate;

protected:
	UFUNCTION()
	void OnBtnClicked();

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Player;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Name;

private:
	FString TargetPlayerName;
};
