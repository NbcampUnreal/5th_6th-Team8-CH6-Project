// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWPostGameEntry.h"
#include "Components/TextBlock.h"

void UPTWPostGameEntry::SetEntryData(int32 InRank, const FPTWPlayerData& InData, const FPTWPlayerRoundData& InRoundData, FString SteamName, bool bIsMe)
{
	Super::SetEntryData(InRank, InData, InRoundData, SteamName, bIsMe);

	if (Text_WinPoints) Text_WinPoints->SetText(FText::AsNumber(InData.TotalWinPoints));
	if (Text_Gold) Text_Gold->SetText(FText::AsNumber(InData.Gold));
}
