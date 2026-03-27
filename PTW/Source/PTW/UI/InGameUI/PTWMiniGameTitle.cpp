// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWMiniGameTitle.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"

#include "MiniGame/PTWMiniGameMapRow.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

/* 로그용 */
#include "CoreFramework/PTWPlayerState.h"

//void UPTWMiniGameTitle::InitPS()
//{
//	TryBindGameState();
//}

void UPTWMiniGameTitle::UpdateTitleByPhase(EPTWGamePhase CurrentGamePhase)
{
	if (!TitleText) return;

	switch (CurrentGamePhase)
	{
	case EPTWGamePhase::PreGameLobby:
		TitleText->SetText(FText::FromString(TEXT("시작 대기중...")));
		break;

	case EPTWGamePhase::PostGameLobby:
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

void UPTWMiniGameTitle::UpdateTitleByRoulette(const FPTWRouletteData& RouletteData)
{
	if (!TitleText) return;

	if (RouletteData.CurrentPhase == EPTWRoulettePhase::RoundEventRoulette ||
		RouletteData.CurrentPhase == EPTWRoulettePhase::Finished)
	{
		TitleText->SetText(GetMapDisplayName(RouletteData.MapRowName));
	}
	else if (RouletteData.CurrentPhase == EPTWRoulettePhase::None ||
		RouletteData.CurrentPhase == EPTWRoulettePhase::MapRoulette)
	{
		TitleText->SetText(FText::FromString(TEXT("미니게임 선택중...")));
	}
}

void UPTWMiniGameTitle::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWMiniGameTitle::NativeDestruct()
{
	Super::NativeDestruct();
}

//void UPTWMiniGameTitle::InitializeTitleState()
//{
//	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
//	if (!GS || !TitleText) return;
//
//	BindGSDelegates();
//
//	// 현재 GamePhase 확인
//	EPTWGamePhase CurrentPhase = GS->GetCurrentGamePhase();
//
//	HandleGamePhaseChanged(CurrentPhase);
//}

//void UPTWMiniGameTitle::TryBindGameState()
//{
//	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
//
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	if (!GS)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 TryBindGameState 재시도."),
//			PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
//		GetWorld()->GetTimerManager().SetTimer(
//			GameStateBindTimerHandle,
//			this,
//			&UPTWMiniGameTitle::TryBindGameState,
//			0.1f, // 0.1초 간격으로 체크
//			false
//		);
//		return;
//	}
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 TryBindGameState 성공."),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
//	// 성공했다면 타이머 클리어 후 초기화 진행
//	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);
//
//	InitializeTitleState();
//}
//
//void UPTWMiniGameTitle::BindGSDelegates()
//{
//	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
//	if (!GS || !TitleText) return;
//
//	UnBindGSDelegates();
//
//	GS->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);
//	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
//
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 BindGSDelegates 성공 / GS Ptr: %p"),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"), GS);
//
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] IsBound: %d"),
//		GS->OnRoulettePhaseChanged.IsAlreadyBound(this, &ThisClass::HandleRoulettePhaseChanged));
//}
//
//void UPTWMiniGameTitle::UnBindGSDelegates()
//{
//	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
//	{
//		GS->OnGamePhaseChanged.RemoveDynamic(this, &ThisClass::HandleGamePhaseChanged);
//		GS->OnRoulettePhaseChanged.RemoveDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
//	}
//
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 UnBindGSDelegates 성공."),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//}

//void UPTWMiniGameTitle::HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase)
//{
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 HandleGamePhaseChanged 함수 호출됨."),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
	//switch (CurrentGamePhase)
	//{
	//case EPTWGamePhase::PreGameLobby:
	//	TitleText->SetText(FText::FromString(TEXT("시작 대기중...")));
	//	break;

	//case EPTWGamePhase::PostGameLobby:
	//	// 룰렛 상태 초기화
	//	InitializeRouletteData();
	//	break;

	//case EPTWGamePhase::Loading:
	//	TitleText->SetText(FText::FromString(TEXT("로딩중...")));
	//	break;

	//case EPTWGamePhase::MiniGame:
	//{
	//	// 미니게임 시작 시 저장된 맵 이름을 가져와 출력
	//	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	//	if (GS)
	//	{
	//		TitleText->SetText(GetMapDisplayName(GS->GetRouletteData().MapRowName));
	//	}
	//}
	//break;

	//case EPTWGamePhase::MiniGameResult:
	//case EPTWGamePhase::GameResult:
	//	break;

	//default:
	//	break;
	//}
//}
//
//void UPTWMiniGameTitle::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
//{
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 HandleRoulettePhaseChanged 함수 호출됨."),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
//	if (RouletteData.CurrentPhase == EPTWRoulettePhase::RoundEventRoulette ||
//		RouletteData.CurrentPhase == EPTWRoulettePhase::Finished)
//	{
//		TitleText->SetText(GetMapDisplayName(RouletteData.MapRowName));
//	}
//	else if (RouletteData.CurrentPhase == EPTWRoulettePhase::None ||
//		RouletteData.CurrentPhase == EPTWRoulettePhase::MapRoulette)
//	{
//		TitleText->SetText(FText::FromString(TEXT("미니게임 선택중...")));
//	}
//}
//
//void UPTWMiniGameTitle::InitializeRouletteData()
//{
//	/* 로그용 */
//	APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>();
//
//	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
//	if (!GS)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 InitializeRouletteData : !GameState."),
//			PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
//		return;
//	}
//	UE_LOG(LogTemp, Warning, TEXT("[PTWMiniGameTitle] %s 플레이어 InitializeRouletteData 성공."),
//		PS ? *PS->GetPlayerName() : TEXT("Unknown"));
//
//	FPTWRouletteData CurrentData = GS->GetRouletteData();
//
//	HandleRoulettePhaseChanged(CurrentData);
//}
//
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
