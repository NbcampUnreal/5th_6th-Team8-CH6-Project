// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRankingEntry.generated.h"

class UTextBlock;
struct FPTWPlayerData;

/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(int32 InRank, const FPTWPlayerData& InData, FString SteamName, bool bIsMe);

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
