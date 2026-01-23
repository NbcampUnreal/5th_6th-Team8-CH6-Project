// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRankingEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(int32 InRank, const FString& InPlayerName, int32 InWinPoints, float InGold);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Rank;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Name;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_WinPoints;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Gold;
};
