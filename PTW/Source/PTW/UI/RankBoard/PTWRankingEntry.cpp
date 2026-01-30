// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingEntry.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerData.h"

void UPTWRankingEntry::SetEntryData(int32 InRank, const FPTWPlayerData& InData, const FPTWPlayerRoundData& InRoundData, FString SteamName, bool bIsMe)
{
	if (Text_Rank) Text_Rank->SetText(FText::AsNumber(InRank));

	if (Text_Name)
	{
		FString DisplayName = InData.PlayerName.IsEmpty() ? SteamName : InData.PlayerName;
		Text_Name->SetText(FText::FromString(DisplayName));
		if (bIsMe) Text_Name->SetColorAndOpacity(FLinearColor::Yellow);
	}
}
