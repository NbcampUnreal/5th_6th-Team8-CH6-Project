// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "PTWLobbyItemManager.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Debug/PTWLogCategorys.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/Manager/PTWRoundEventManager.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"
#include "System/PTWSessionSubsystem.h"
#include "System/Session/PTWSessionConfig.h"
#include "GameFramework/SpectatorPawn.h"

APTWLobbyGameMode::APTWLobbyGameMode()
{
	RoundEventManager = CreateDefaultSubobject<UPTWRoundEventManager>(TEXT("RoundEventManager"));
	LobbyItemManager = CreateDefaultSubobject<UPTWLobbyItemManager>(TEXT("LobbyItemManager"));

	bStartPlayersAsSpectators = false;
}

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
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
		{
			GameFlowRule.MaxPlayers = SessionSubsystem->GetMaxPlayers();
			GameFlowRule.MaxRound = SessionSubsystem->GetMaxRounds();
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
			//TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");
		}
	}
}

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (RoundEventManager)
	{
		RoundEventManager->OnRouletteFinished.AddDynamic(this, &APTWLobbyGameMode::OnRouletteFinished);
	}

	if (!PTWGameState) return;
	
	LobbyItemManager = NewObject<UPTWLobbyItemManager>(this);
	LobbyItemManager->InitLobbyItemManager(LobbyItemDataTable, PTWGameState);
	
	if (PTWGameState->GetCurrentGamePhase() != EPTWGamePhase::PreGameLobby)
	{
#if WITH_EDITOR
		GetWorldTimerManager().SetTimer(TestTimer, this, &APTWLobbyGameMode::StartGameLobby, 2.f, false);
		UE_LOG(Log_LobbyGameMode, Warning, TEXT("현재 빌드: WITH_EDITOR"));

#else
		GetWorldTimerManager().SetTimer(TestTimer, this, &APTWLobbyGameMode::StartGameLobby, 10.f, false);
		UE_LOG(Log_LobbyGameMode, Warning, TEXT("현재 빌드: Not WITH_EDITOR"));
#endif
	}
	
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

			if (!bIsGameStarted)
			{
				StartGameLobby();
			}
			
			return;
		}
		
		if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
		{
			StartTimer(GameFlowRule.WaitingTime);
		}
	}
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	// 로그 아웃하면 gamestate portal 부분 수정
}

void APTWLobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	UE_LOG(Log_LobbyGameMode, Warning, TEXT("[로비게임모드] HandleSeamlessTravelPlayer: %s"), *C->GetName());
	
	Super::HandleSeamlessTravelPlayer(C);
	
	PlayerReadyToPlay(Cast<APTWPlayerController>(C));
}
void APTWLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;

	if (!LobbyItemManager) return;
	
	// 데이터 초기화 및 골드 지급
	PTWPlayerState->ResetInventoryItemId();
	AddGold(PTWPlayerState, RoundClearBonusGold + LobbyItemManager->TakeSavingsReward(PTWPlayerState));
}

void APTWLobbyGameMode::PlayerReadyToPlay(APlayerController* Controller)
{
	Super::PlayerReadyToPlay(Controller);

	UE_LOG(Log_LobbyGameMode, Warning, TEXT("[Lobby] PlayerReadyToPlay: %s, %d/%d"), *Controller->GetName(),ReadyPlayer, AllPlayer);
	
	if (!IsValid(PTWGameState) || !Controller) return;
	
	APTWPlayerState* PTWPlayerState = Controller->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;
	
	PTWPlayerState->bIsReadyToPlay = true;
	
	if (ReadyPlayer >= AllPlayer)
	{
		if (bAllPlayerReady) return;
		bAllPlayerReady = true;
		
		GetWorldTimerManager().ClearTimer(LoadingDelayTimer);
		GetWorldTimerManager().SetTimer(LoadingDelayTimer, this, &APTWLobbyGameMode::StartGameLobby, 3.f);
	}
}


void APTWLobbyGameMode::StartGameLobby()
{
	if (bIsGameStarted) return;
	bIsGameStarted = true;
	if (!IsValid(PTWGameState)) return;

	GetWorldTimerManager().ClearTimer(TestTimer);
	ClearTimer();
	// 최대 라운드에 도달 하면 게임 종료
	if (PTWGameState->GetCurrentRound() >= GameFlowRule.MaxRound)
	{
		EndGame();
		
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Start Game Lobby"));
	
	PTWGameState->AdvanceRound();
	
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
		
		//SetInputBlock(false);
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

	// 관전 상태일 때만 처리
	if (!PC->PlayerState || !PC->PlayerState->IsSpectator()) return;

	PC->ChangeState(NAME_Playing);
	PC->ClientGotoState(NAME_Playing);

	PC->PlayerState->SetIsSpectator(false);
	PC->PlayerState->SetIsOnlyASpectator(false);
}

void APTWLobbyGameMode::StartRoulette()
{
	//SelectedRandomMap();
	//SelectedRandomEvent();
	
	RoundEventManager->StartRouletteSequence();
}


void APTWLobbyGameMode::OnRouletteFinished(FName SelectedMapName)
{
	PrepareAllPlayersLoadingScreen(ELoadingScreenType::MiniGame, SelectedMapName);

	TravelLevelName = RoundEventManager->TravelLevelName;
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

void APTWLobbyGameMode::ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget)
{
	if (!LobbyItemManager) return;
	
	LobbyItemManager->ApplyLobbyItem(Buyer, ItemId, WinTarget);
}

void APTWLobbyGameMode::AddChaosItemEntry(const FPTWChaosItemEntry& Entry)
{
	if (!PTWGameState) return;

	PTWGameState->AddChaosItemEntry(Entry);
}

void APTWLobbyGameMode::AddGold(APTWPlayerState* PlayerState, int32 Amount)
{
	FPTWPlayerData PlayerData = PlayerState->GetPlayerData();
	PlayerData.Gold += Amount; // 적금 골드를 받을 수 있으면 지급 골드에 추가
	PlayerState->SetPlayerData(PlayerData);
}


