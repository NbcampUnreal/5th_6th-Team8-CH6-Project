// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingBoard.h"
#include "PTWRankingEntry.h"
#include "Components/VerticalBox.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/GameStateBase.h"

void UPTWRankingBoard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UpdateRanking();
}

void UPTWRankingBoard::NativeDestruct()
{
	Super::NativeDestruct();

}

void UPTWRankingBoard::UpdateRanking()
{
	if (!RankingList || !RankingEntryClass) return;

	RankingList->ClearChildren();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	TArray<APTWPlayerState*> Players;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS))
		{
			Players.Add(PTWPS);
		}
	}

	// 승점 기준 정렬
	Players.Sort([](const APTWPlayerState& A, const APTWPlayerState& B)
		{
			return A.GetPlayerData().TotalWinPoints >
				B.GetPlayerData().TotalWinPoints;
		});

	int32 Rank = 1;

	for (APTWPlayerState* PS : Players)
	{
		const FPTWPlayerData& Data = PS->GetPlayerData();

		TObjectPtr<UPTWRankingEntry> Entry =
			CreateWidget<UPTWRankingEntry>(this, RankingEntryClass);

		if (!Entry) continue;

		Entry->SetEntry(
			Rank++,
			Data.PlayerName,
			Data.TotalWinPoints,
			Data.Gold
		);

		RankingList->AddChild(Entry);
	}
}
