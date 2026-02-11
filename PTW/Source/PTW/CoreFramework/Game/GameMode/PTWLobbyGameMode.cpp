// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "MiniGame/Data/PTWRoundEvent.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"
#include "System/Shop/PTWShopSubsystem.h"


void APTWLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	if (UPTWGameInstance* PTWGameInstance = Cast<UPTWGameInstance>(GetGameInstance()))
	{
		if (PTWGameInstance->bIsFirstLobby == true && bSkipFirstLobby == false)
		{
			bIsFirstLobby = true;	
			PTWGameInstance->bIsFirstLobby = false;
		}
		else
		{
			bIsFirstLobby = false;
			PTWGameInstance->bIsFirstLobby = false;
		}
	}
	
}

void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();
	
	if (IsValid(PTWGameState))
	{
		if (bIsFirstLobby == true)
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::PreGameLobby);
			TravelLevelName = TEXT("/Game/_PTW/Maps/Lobby");
		}
		else
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::Loading);
			TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");
			PTWGameState->AdvanceRound(); // 라운드 증가
		}
	}
}

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//접속하면 무적 상태로 변경 해야 함
	
	if (!IsValid(PTWGameState)) return;
	
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		AddGold(NewPlayer);

		if (GameFlowRule.MinPlayersToStart <= PTWGameState->PlayerArray.Num() &&
			GameFlowRule.bAutoStartWhenMinPlayersMet)
		{
			if (bIsGameStart) return;
			
			// gameinstance 에 현재 로비에 있는 플레이어 수 저장
			if (UPTWGameInstance* PTWGameInstance = Cast<UPTWGameInstance>(GetGameInstance()))
			{
				PTWGameInstance->CurrentPlayerCount = PTWGameState->PlayerArray.Num();
			}
			
			StartGameLobby();

			return;
		}
		
		if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
		{
			StartTimer(GameFlowRule.WaitingTime);
		}
	}
}

void APTWLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!IsValid(PTWGameState)) return;

	// 미니 게임 끝나면 인벤토리 비워주지만 로비 이동 후에 한번더 초기화
	APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;

	PTWPlayerState->ResetInventoryItemId();
	RestartPlayer(NewPlayer);
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PostGameLobby)
	{
		// 로딩 UI 
		if (PTWGameState->PlayerArray.Num() == 1) // 임시 설정
		{
			// 플레이 중인 모든 플레이어 접속 중이면 로딩 UI 해제
		}

		StartGameLobby();
	}
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (!IsValid(PTWGameState)) return;

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PostGameLobby)
	{
		if (UPTWGameInstance* PTWGameInstance = Cast<UPTWGameInstance>(GetGameInstance()))
		{
			PTWGameInstance->CurrentPlayerCount = PTWGameState->PlayerArray.Num();
		}
	}

	// 로그 아웃하면 gamestate portal 부분 수정
}

void APTWLobbyGameMode::StartGameLobby()
{
	ClearTimer();

	bIsGameStart = true;
	
	if (!IsValid(PTWGameState)) return;

	// 대기 로비에서 게임 로비로 이동 했을 때 모든 플레이어 시작 위치로 이동
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		
		for (APlayerState* PlayerState : PTWGameState->PlayerArray)
		{
			if (!PlayerState) continue;
			
			AController* Controller = PlayerState->GetOwningController();
			if (!Controller) continue;
			
			MovePlayerToStart(Controller);
		}
	}
	
	PTWGameState->AdvanceRound();
	PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);
	
	// 게임 로비 진입 5초 후 룰렛 시작
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		StartTimer(GameFlowRule.NextMiniGameWaitTime);

		FTimerHandle RouletteDelayTimerHandle;
		GetWorldTimerManager().SetTimer(RouletteDelayTimerHandle, this, &APTWLobbyGameMode::StartRoulette, GameFlowRule.RouletteDelay);
	}
}

void APTWLobbyGameMode::EndTimer()
{
	if (!PTWGameState) return;

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		StartGameLobby();
		
		return;
	}

	Super::EndTimer();
}

void APTWLobbyGameMode::AddGold(APlayerController* NewPlayer)
{
	int32 RandomGold = 500;

	if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
		PlayerData.Gold = RandomGold;
		PTWPlayerState->SetPlayerData(PlayerData);

		UE_LOG(LogTemp, Warning, TEXT("RandomGold: %d"), RandomGold);
	}

	// 나중에 preLobby가 아닌 postLobby일 때 한번에 모든 플레이어에게 골드 지급하는 식으로 변경해야함
	// 하드 코딩 되있는 부분도 변수로 만들어서 bp에서 수정 가능하게 변경
}

void APTWLobbyGameMode::StartRoulette()
{
	SelectedRandomMap();
	SelectedRandomEvent();
	
	StartMapRoulette();
}

void APTWLobbyGameMode::SelectedRandomMap()
{
	if (!PTWGameState) return;
	if (!MiniGameMapTable) return;
	
	TArray<FName> SelectableRowNames = GetSelectableMapRowNames();

	if (SelectableRowNames.Num() == 0) return;
	
	const int32 RandomIndex = FMath::RandRange(0, SelectableRowNames.Num()-1);
	const FName SelectedRowName = SelectableRowNames[RandomIndex];

	const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(SelectedRowName, TEXT("Map"));

	if (Row)
	{
		TravelLevelName = Row->Map.ToSoftObjectPath().GetLongPackageName();
		MapTag = Row->MiniGameTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->DisplayName.ToString(), false);
	}

	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.MapRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData); // GameState에 전달
}

void APTWLobbyGameMode::SelectedRandomEvent()
{
	if (!PTWGameState) return;
	if (!LobbyRoundEventTable) return;

	TArray<FName> RowNames = LobbyRoundEventTable->GetRowNames();

	if (RowNames.Num() ==0) return;

	const int32 RandomIndex = FMath::RandRange(0, RowNames.Num()-1);
	const FName SelectedRowName = RowNames[RandomIndex];

	const FPTWRoundEventRow* Row = LobbyRoundEventTable->FindRow<FPTWRoundEventRow>(SelectedRowName, TEXT("Round Event"));
	
	if (Row)
	{
		EventTag = Row->EventTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->EventName.ToString());
	}
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.EventRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData);
}

void APTWLobbyGameMode::StartMapRoulette()
{
	if (!PTWGameState) return;

	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::MapRoulette;
	RouletteData.RouletteDuration = 5.f;
	PTWGameState->SetRouletteData(RouletteData);

	GetWorldTimerManager().SetTimer(RouletteTimer, this, &APTWLobbyGameMode::StartRoundEventRoulette, 5.f);
	
}

void APTWLobbyGameMode::StartRoundEventRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::RoundEventRoulette;
	RouletteData.RouletteDuration = 5.f;
	PTWGameState->SetRouletteData(RouletteData);

	GetWorldTimerManager().SetTimer(RouletteTimer, this, &APTWLobbyGameMode::EndRoulette, 5.f);
}

void APTWLobbyGameMode::EndRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::Finished;
	PTWGameState->SetRouletteData(RouletteData);
	
	if (UPTWShopSubsystem* ShopSubsystem = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		ShopSubsystem->InitializeShopsForRound(MapTag, EventTag);
	}
}

TArray<FName> APTWLobbyGameMode::GetSelectableMapRowNames()
{
	TArray<FName> SelectableRowNames;
	
	if (!PTWGameState)
	{
		return SelectableRowNames;
	}
	if (!MiniGameMapTable)
	{
		return SelectableRowNames;
	}

	int32 PlayerCount = PTWGameState->PlayerArray.Num();
	TArray<FName> RowNames = MiniGameMapTable->GetRowNames();

	SelectableRowNames.Reserve(RowNames.Num());

	// 모든 Row를 순회하면서 선택 가능 여부를 검사
	for (const FName& RowName : RowNames)
	{
		const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(RowName, TEXT("Map"));

		if (!Row) continue;
		
		if (Row->MinPlayers <= PlayerCount && Row->MaxPlayers >= PlayerCount)
		{
			// 조건을 만족하면 선택 가능한 후보로 추가
			SelectableRowNames.Add(RowName);
		}
	}
	// 조건을 만족하는 모든 맵 RowName 반환
	return SelectableRowNames;
}


