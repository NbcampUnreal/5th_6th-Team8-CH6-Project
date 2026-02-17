// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "MiniGame/Data/PTWRoundEvent.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"
#include "System/PTWSessionSubsystem.h"
#include "System/Session/SessionConfig.h"
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
	GameFlowRule.MaxPlayers = UGameplayStatics::GetIntOption(Options, SessionKey::MaxPlayers.ToString(), 16);
	GameFlowRule.MaxRound  = UGameplayStatics::GetIntOption(Options, SessionKey::MaxRounds.ToString(), 5);
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
		if (GameFlowRule.MinPlayersToStart <= PTWGameState->PlayerArray.Num() &&
			GameFlowRule.bAutoStartWhenMinPlayersMet)
		{
			if (bAllPlayerReady) return;
			
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
	
	// 골드 지급
	FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
	PlayerData.Gold += RoundClearBonusGold;
	PTWPlayerState->SetPlayerData(PlayerData);

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::Loading)
	{
		//SetInputBlock(true);
		
		if (AllPlayer <= PTWGameState->PlayerArray.Num())
		{
			if (bAllPlayerReady) return;
			bAllPlayerReady = true;
			FTimerHandle LoadingDealyTimer;
			GetWorldTimerManager().SetTimer(LoadingDealyTimer, this, &APTWLobbyGameMode::StartGameLobby, 3.f);
		}
	}

	RestartPlayer(NewPlayer);
}

void APTWLobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	APlayerController* PlayerController  = Cast<APlayerController>(C);
	if (!PlayerController) return;
	
	ExitSpectorMode(PlayerController);
	RestartPlayer(PlayerController);
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (!IsValid(PTWGameState)) return;

	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->DecreasePlayerCount();
	}

	// 로그 아웃하면 gamestate portal 부분 수정
}


void APTWLobbyGameMode::StartGameLobby()
{
	ClearTimer();

	// 최대 라운드에 도달 하면 게임 종료
	if (PTWGameState->GetCurrentRound() >= GameFlowRule.MaxRound)
	{
		EndGame();
		
		return;
	}
	
	PTWGameState->AdvanceRound();
	
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
	
	
	PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);

	for (APlayerState* PS : PTWGameState->PlayerArray)
	{
		AController* PC = PS->GetPlayerController();
		if (!PC) continue;
		
		SetInputBlock(false);
	}
	
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

void APTWLobbyGameMode::ExitSpectorMode(AController* Controller)
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	// 1. 관전 상태 및 대기 상태 강제 종료
	PC->ChangeState(NAME_Playing);
	PC->ClientGotoState(NAME_Playing);
	
	// 2. PlayerState 플래그 초기화
	if (PC->PlayerState)
	{
		PC->PlayerState->SetIsSpectator(false);
		PC->PlayerState->SetIsOnlyASpectator(false);
	}
}

void APTWLobbyGameMode::StartRoulette()
{
	SelectedRandomMap();
	SelectedRandomEvent();
	
	StartMapRoulette();
}

void APTWLobbyGameMode::EndGame()
{
	if (!PTWGameState) return;

	PTWGameState->SetCurrentPhase(EPTWGamePhase::GameResult);

	FTimerHandle EndGameTimer;
	GetWorldTimerManager().SetTimer(EndGameTimer, this, &APTWLobbyGameMode::ReturnToMainMenu, 15.f);
}

void APTWLobbyGameMode::ReturnToMainMenu()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			if (!PC->IsLocalController())
			{
				PC->Client_OpenMainMenu();
			}
		}
	}
	// 리슨 서버일 경우 호스트는 따로 구현
	UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>();
	if (!GI) return;
	
	if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
	{
		SessionSubsystem->LeaveGameSession();
	}
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

	FName SelectedMapName = PTWGameState->GetRouletteData().MapRowName;
	PrepareAllPlayersLoadingScreen(ELoadingScreenType::MiniGame, SelectedMapName);
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


