// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RankBoard/PTWRankingBoard.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

#include "GameFramework/GameStateBase.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWPlayerData.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
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
	if (!RankingList) return;

	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS) return;

	/* 현재 페이즈에 따라 생성할 위젯 클래스 선택 */
	TSubclassOf<UUserWidget> SelectedHeaderClass = nullptr;
	TSubclassOf<UPTWRankingEntry> SelectedEntryClass = nullptr;

	int32 Round = GS->GetCurrentRound();
	FString TitleString = FString::Printf(TEXT("ROUND %d "), Round);

	switch (GS->GetCurrentGamePhase())
	{
	case EPTWGamePhase::PreGameLobby:
		Text_GameTitle->SetVisibility(ESlateVisibility::Collapsed);
		SelectedHeaderClass = PreGameHeaderClass;
		SelectedEntryClass = PreGameEntryClass;
		break;
	case EPTWGamePhase::MiniGame:
		Text_GameTitle->SetText(FText::FromString(TitleString));
		Text_GameTitle->SetVisibility(ESlateVisibility::Visible);
		SelectedHeaderClass = MiniGameHeaderClass;
		SelectedEntryClass = MiniGameEntryClass;
		break;
	case EPTWGamePhase::PostGameLobby:
		Text_GameTitle->SetVisibility(ESlateVisibility::Collapsed);
		SelectedHeaderClass = PostGameHeaderClass;
		SelectedEntryClass = PostGameEntryClass;
		break;
	}

	if (!SelectedEntryClass) return;

	RankingList->ClearChildren();

	/* 상단 헤더 위젯 생성 */
	if (SelectedHeaderClass)
	{
		UUserWidget* HeaderWidget = CreateWidget<UUserWidget>(this, SelectedHeaderClass);
		if (HeaderWidget)
		{
			RankingList->AddChild(HeaderWidget);
		}
	}

	const TArray<APTWPlayerState*>& SortedPlayerStates = GS->GetRankedPlayers();

	/* 내 PlayerState */
	APTWPlayerState* MyPlayerState = GetOwningPlayerState<APTWPlayerState>();

	/* 랭킹 순으로 위젯 생성 */
	int32 CurrentRank = 1;      // 표시될 순위
	int32 TotalProcessed = 0;   // 처리된 전체 인원 수

	FPTWPlayerData PreviousData; // 이전 플레이어의 데이터 저장용

	for (int32 i = 0; i < SortedPlayerStates.Num(); ++i)
	{
		APTWPlayerState* PS = SortedPlayerStates[i];
		if (!PS) continue; // 방어 코드
		const FPTWPlayerData& CurrentData = PS->GetPlayerData();
		TotalProcessed++;

		if (i > 0)
		{
			// 이전 사람과 승점 및 골드가 모두 같은지 확인
			bool bIsTie = (CurrentData.TotalWinPoints == PreviousData.TotalWinPoints) && (CurrentData.Gold == PreviousData.Gold);

			if (!bIsTie)
			{
				CurrentRank = TotalProcessed;
			}
		}

		// 이전 데이터 업데이트
		PreviousData = CurrentData;

		// 위젯 생성 및 데이터 주입
		UPTWRankingEntry* Entry = CreateWidget<UPTWRankingEntry>(this, SelectedEntryClass);
		if (Entry)
		{
			Entry->SetEntryData(
				CurrentRank, // 계산된 공동 순위 전달
				CurrentData,
				PS->GetPlayerRoundData(),
				PS->GetPlayerName(),
				(PS == GetOwningPlayerState<APTWPlayerState>())
			);
			RankingList->AddChild(Entry);
		}
	}
}

void UPTWRankingBoard::OnPlayerDataChanged(const FPTWPlayerData& NewData)
{
	UpdateRanking();
}
