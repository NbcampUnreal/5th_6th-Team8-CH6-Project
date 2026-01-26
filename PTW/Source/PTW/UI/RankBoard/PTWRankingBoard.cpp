// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingBoard.h"
#include "GameFramework/GameStateBase.h"
#include "Components/VerticalBox.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWPlayerData.h"
#include "PTWRankingEntry.h"

void UPTWRankingBoard::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerStates();
	UpdateRanking();
}

void UPTWRankingBoard::NativeDestruct()
{
	UnbindPlayerStates();

	Super::NativeDestruct();
}

void UPTWRankingBoard::BindPlayerStates()
{
	if (!GetWorld()) return;

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS))
		{
			PTWPS->OnPlayerDataUpdated.AddDynamic(this, &UPTWRankingBoard::OnPlayerDataChanged);

			CachedPlayerStates.Add(PTWPS);
		}
	}
}

void UPTWRankingBoard::UnbindPlayerStates()
{
	for (APTWPlayerState* PS : CachedPlayerStates)
	{
		if (PS)
		{
			PS->OnPlayerDataUpdated.RemoveAll(this);
		}
	}
	CachedPlayerStates.Empty();
}

void UPTWRankingBoard::UpdateRanking()
{
	if (!RankingList || !RankingEntryClass) return;

	RankingList->ClearChildren();

	/* 현재 GameState에 있는 모든 PlayerState를 다시 확인 */
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS))
			{
				// 이미 바인딩 되어있는지 확인 후 없으면 추가 (델리게이트 중복 방지)
				if (!CachedPlayerStates.Contains(PTWPS))
				{
					PTWPS->OnPlayerDataUpdated.AddDynamic(this, &UPTWRankingBoard::OnPlayerDataChanged);
					CachedPlayerStates.Add(PTWPS);
				}
			}
		}
	}

	/* 정렬용 배열 */
	TArray<APTWPlayerState*> SortedPlayerStates;
	for (APTWPlayerState* PS : CachedPlayerStates)
	{
		if (PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerName: [%s]"),
				*PS->GetPlayerData().PlayerName);

			UE_LOG(LogTemp, Warning, TEXT("PlayerName: [%s]"),
				*PS->GetPlayerName());

			SortedPlayerStates.Add(PS);
		}
	}

	/* 정렬: 1. 승점  2. 골드 */
	SortedPlayerStates.Sort(
		[](const APTWPlayerState& A, const APTWPlayerState& B)
		{
			const FPTWPlayerData& DA = A.GetPlayerData();
			const FPTWPlayerData& DB = B.GetPlayerData();

			if (DA.TotalWinPoints != DB.TotalWinPoints)
			{
				return DA.TotalWinPoints > DB.TotalWinPoints;
			}
			return DA.Gold > DB.Gold;
		}
	);

	/* 내 PlayerState */
	APTWPlayerState* MyPlayerState = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		MyPlayerState = PC->GetPlayerState<APTWPlayerState>();
	}

	int32 Rank = 1;

	for (APTWPlayerState* PS : SortedPlayerStates)
	{
		UPTWRankingEntry* Entry = CreateWidget<UPTWRankingEntry>(this, RankingEntryClass);

		if (!Entry) continue;

		const bool bIsMe = (PS == MyPlayerState);

		FString SteamName = *PS->GetPlayerName();

		Entry->SetEntry(Rank++, PS->GetPlayerData(), SteamName, bIsMe);

		RankingList->AddChild(Entry);
	}
}

void UPTWRankingBoard::OnPlayerDataChanged(const FPTWPlayerData& NewData)
{
	UpdateRanking();
}
