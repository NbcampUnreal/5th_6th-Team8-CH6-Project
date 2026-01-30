// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RankBoard/PTWRankingEntry.h"
#include "PTWMiniGameEntry.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameEntry : public UPTWRankingEntry
{
	GENERATED_BODY()

public:
	virtual void SetEntryData(int32 InRank, const FPTWPlayerData& InData, const FPTWPlayerRoundData& InRoundData, FString SteamName, bool bIsMe) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_WinPoints;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_KDA;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_MiniGameName; // 현재 미니게임 이름 표시용
};
