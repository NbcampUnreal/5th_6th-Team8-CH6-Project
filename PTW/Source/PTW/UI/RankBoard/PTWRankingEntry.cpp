// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingEntry.h"
#include "Components/TextBlock.h"

void UPTWRankingEntry::SetEntry(
	int32 InRank,
	const FString& InPlayerName,
	int32 InWinPoints,
	float InGold)
{
	if (Text_Rank)
		Text_Rank->SetText(FText::AsNumber(InRank));

	if (Text_Name)
		Text_Name->SetText(FText::FromString(InPlayerName));

	if (Text_WinPoints)
		Text_WinPoints->SetText(FText::AsNumber(InWinPoints));

	if (Text_Gold)
	{
		Text_Gold->SetText(
			FText::AsNumber(FMath::FloorToInt(InGold))
		);
	}
}

void UPTWRankingEntry::NativeConstruct()
{
	Super::NativeConstruct();
}
