// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingEntry.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerData.h"

void UPTWRankingEntry::SetEntry(int32 InRank, const FPTWPlayerData& InData, bool bIsMe)
{
	if (Text_Rank)
		Text_Rank->SetText(FText::AsNumber(InRank));

	if (Text_Name)
		Text_Name->SetText(FText::FromString(InData.PlayerName));

	if (Text_WinPoints)
		Text_WinPoints->SetText(FText::AsNumber(InData.TotalWinPoints));

	if (Text_Gold)
		Text_Gold->SetText(FText::AsNumber(InData.Gold));
}

void UPTWRankingEntry::NativeConstruct()
{
	Super::NativeConstruct();
}
