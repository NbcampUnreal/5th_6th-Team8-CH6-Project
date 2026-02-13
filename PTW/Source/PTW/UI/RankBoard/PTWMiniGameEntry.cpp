// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWMiniGameEntry.h"
#include "Components/TextBlock.h"

void UPTWMiniGameEntry::SetEntryData(int32 InRank, const FPTWPlayerData& InData, const FPTWPlayerRoundData& InRoundData, FString SteamName, bool bIsMe)
{
	Super::SetEntryData(InRank, InData, InRoundData, SteamName, bIsMe);

	if (Text_WinPoints) Text_WinPoints->SetText(FText::AsNumber(InRoundData.Score));

	if (Text_KDA)
	{
		FString KDAStr = FString::Printf(TEXT("%d / %d"), InRoundData.KillCount, InRoundData.DeathCount);
		Text_KDA->SetText(FText::FromString(KDAStr));
	}
}
