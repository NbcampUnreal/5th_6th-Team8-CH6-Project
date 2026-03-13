// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameTitle.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"

#include "MiniGame/PTWMiniGameMapRow.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWMiniGameTitle::NativeConstruct()
{
	Super::NativeConstruct();

	TryBindGameState();
}

void UPTWMiniGameTitle::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnGamePhaseChanged.RemoveDynamic(this, &ThisClass::HandleGamePhaseChanged);
		GS->OnRoulettePhaseChanged.RemoveDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
	}

	Super::NativeDestruct();
}

void UPTWMiniGameTitle::InitializeTitleState()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS || !TitleText) return;

	GS->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);

	// 현재 GamePhase 확인
	EPTWGamePhase CurrentPhase = GS->GetCurrentGamePhase();

	HandleGamePhaseChanged(GS->GetCurrentGamePhase());
}

void UPTWMiniGameTitle::TryBindGameState()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();

	if (!GS)
	{
		GetWorld()->GetTimerManager().SetTimer(
			GameStateBindTimerHandle,
			this,
			&UPTWMiniGameTitle::TryBindGameState,
			0.1f, // 0.1초 간격으로 체크
			false
		);
		return;
	}

	// 성공했다면 타이머 클리어 후 초기화 진행
	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);

	InitializeTitleState();
}

void UPTWMiniGameTitle::HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase)
{
	switch (CurrentGamePhase)
	{
	case EPTWGamePhase::PreGameLobby:
		TitleText->SetText(FText::FromString(TEXT("시작 대기중...")));
		break;

	case EPTWGamePhase::PostGameLobby:
		// 룰렛 구독 로직 실행
		StartRouletteSubscription();
		break;

	case EPTWGamePhase::Loading:
		TitleText->SetText(FText::FromString(TEXT("로딩중...")));
		break;

	case EPTWGamePhase::MiniGame:
	{
		// 미니게임 시작 시 저장된 맵 이름을 가져와 출력
		APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
		if (GS)
		{
			TitleText->SetText(GetMapDisplayName(GS->GetRouletteData().MapRowName));
		}
	}
	break;

	case EPTWGamePhase::MiniGameResult:
	case EPTWGamePhase::GameResult:
		break;

	default:
		break;
	}
}

void UPTWMiniGameTitle::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
{
	if (RouletteData.CurrentPhase == EPTWRoulettePhase::RoundEventRoulette ||
		RouletteData.CurrentPhase == EPTWRoulettePhase::Finished)
	{
		TitleText->SetText(GetMapDisplayName(RouletteData.MapRowName));
	}
}

void UPTWMiniGameTitle::StartRouletteSubscription()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS) return;

	// 룰렛 상태 델리게이트 바인딩
	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);

	FPTWRouletteData CurrentData = GS->GetRouletteData();

	if (CurrentData.CurrentPhase == EPTWRoulettePhase::None ||
		CurrentData.CurrentPhase == EPTWRoulettePhase::MapRoulette)
	{
		TitleText->SetText(FText::FromString(TEXT("미니게임 선택중...")));
	}
	else
	{
		HandleRoulettePhaseChanged(CurrentData);
	}
}

FText UPTWMiniGameTitle::GetMapDisplayName(FName RowName)
{
	if (!MiniGameMapTable || RowName.IsNone())
	{
		return FText::FromString(TEXT("Unknown MiniGame"));
	}

	static const FString ContextString(TEXT("MiniGameTitle"));
	FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(RowName, ContextString);

	if (Row)
	{
		return Row->DisplayName;
	}

	return FText::FromName(RowName);
}
