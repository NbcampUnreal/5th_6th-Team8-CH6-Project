// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "PTWLobbyItemManager.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Debug/PTWLogCategorys.h"
#include "MiniGame/Manager/PTWRoundEventManager.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"
#include "System/PTWSteamSessionSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Actor/PTWResultCharacter.h"

APTWLobbyGameMode::APTWLobbyGameMode()
{
	RoundEventManager = CreateDefaultSubobject<UPTWRoundEventManager>(TEXT("RoundEventManager"));
	LobbyItemManager = CreateDefaultSubobject<UPTWLobbyItemManager>(TEXT("LobbyItemManager"));

	bStartPlayersAsSpectators = false;
}

void APTWLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// 이 부분은 매칭 방식이 바뀌면 필요 없음 
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
		if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
		{
			GameFlowRule.MaxPlayers = SteamSessionSubsystem->GetMaxPlayers();
			GameFlowRule.MaxRound = SteamSessionSubsystem->GetMaxRounds();
		}
	}
}

void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();
	// 이 부분은 매칭 방식이 바뀌면 필요 없음 
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
	
	if (!IsValid(PTWGameState)) return;

	// 이 부분은 매칭 방식이 바뀌면 필요 없음 
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

	// 매칭 방식 바뀌면 추가
	
	APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UPTWGameInstance* GameInstance = GetGameInstance<UPTWGameInstance>();
	if (!GameInstance) return;

	// UPTWScoreSubsystem* ScoreSubsystem = GameInstance->GetSubsystem<UPTWScoreSubsystem>();
	// if (!ScoreSubsystem) return;

	
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
	int32 LobbyItemReward = LobbyItemManager->TakeGoldReward(PTWPlayerState);
	AddGold(PTWPlayerState, RoundClearBonusGold + LobbyItemReward);
	
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
		// 나중에 이부분 함수화 예정
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
	if (!IsValid(PTWGameState) || !IsValid(ScoreSubsystem)) return;
	
	GetWorldTimerManager().ClearTimer(TestTimer);
	ClearTimer();

	PTWGameState->AddLobbyRankingDataMap(ScoreSubsystem->GetConnectedPlayersGameData());
	
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
	
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		StartTimer(GameFlowRule.NextMiniGameWaitTime);
	
		StartRoulette();
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

	//PTWGameState->SetCurrentPhase(EPTWGamePhase::GameResult);

	UWorld* World = GetWorld();
	if (!World) return;

	AActor* ResultCamera = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("ResultCamera"), FoundActors);
	if (FoundActors.Num() > 0) ResultCamera = FoundActors[0];

	TArray<AActor*> WinSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("WinSpot"), WinSpots);

	TArray<AActor*> LoseSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("LoseSpot"), LoseSpots);

	int32 CurrentWinIndex = 0;
	int32 CurrentLoseIndex = 0;

	APTWPlayerState* WinnerPS = nullptr;
	int32 MaxScore = -1;
	int32 MaxGold = -1;

	for (APlayerState* PS : PTWGameState->PlayerArray)
	{
		if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS))
		{
			int32 PlayerScore = PTWPS->GetPlayerRoundData().Score;
			int32 PlayerGold = PTWPS->GetPlayerData().Gold;

			bool bIsNewWinner = false;

			if (WinnerPS == nullptr)
			{
				bIsNewWinner = true;
			}
			else if (PlayerScore > MaxScore)
			{
				bIsNewWinner = true;
			}
			else if (PlayerScore == MaxScore)
			{
				if (PlayerGold > MaxGold)
				{
					bIsNewWinner = true;
				}
			}
			if (bIsNewWinner)
			{
				WinnerPS = PTWPS;
				MaxScore = PlayerScore;
				MaxGold = PlayerGold;
			}
		}
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get());
		if (!PC) continue;

		APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
		if (!PS) continue;

		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		if (APawn* OriginalPawn = PC->GetPawn())
		{
			OriginalPawn->SetActorHiddenInGame(true);
			OriginalPawn->SetActorEnableCollision(false);
		}

		if (ResultCamera)
		{
			FViewTargetTransitionParams Params;
			Params.BlendTime = 1.0f;
			Params.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
			Params.bLockOutgoing = true;

			PC->ClientSetViewTarget(ResultCamera, Params);
		}

		if (ResultCharacterClass)
		{
			bool bIsWinner = (PS == WinnerPS);

			if (WinnerSound) PC->ClientPlaySound(WinnerSound);

			FString PlayerName = PS->GetPlayerData().PlayerName;
			if (PlayerName.IsEmpty())
			{
				PlayerName = PS->GetPlayerName();
			}

			FVector SpawnLoc = FVector::ZeroVector;
			FRotator SpawnRot = FRotator::ZeroRotator;
			bool bValidSpotFound = false;

			if (bIsWinner)
			{
				if (WinSpots.IsValidIndex(CurrentWinIndex))
				{
					SpawnLoc = WinSpots[CurrentWinIndex]->GetActorLocation();
					SpawnRot = WinSpots[CurrentWinIndex]->GetActorRotation();
					CurrentWinIndex++;
					bValidSpotFound = true;

					if (WinnerEffect)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, WinnerEffect, SpawnLoc, SpawnRot);
					}
				}
			}
			else
			{
				if (LoseSpots.IsValidIndex(CurrentLoseIndex))
				{
					SpawnLoc = LoseSpots[CurrentLoseIndex]->GetActorLocation();
					SpawnRot = LoseSpots[CurrentLoseIndex]->GetActorRotation();
					CurrentLoseIndex++;
					bValidSpotFound = true;
				}
			}

			if (bValidSpotFound)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APTWResultCharacter* ResultChar = World->SpawnActor<APTWResultCharacter>(ResultCharacterClass, SpawnLoc, SpawnRot, SpawnParams);

				if (ResultChar)
				{
					ResultChar->InitializeResult(bIsWinner, PlayerName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Lobby Result] 스폰 포인트가 부족합니다!"));
			}
		}
	}

	FTimerHandle EndGameTimer;
	GetWorldTimerManager().SetTimer(EndGameTimer, this, &APTWLobbyGameMode::ReturnToMainMenu, 10.f);
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
	
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamSessionSubsystem->LeaveGameSession();
	}
}

void APTWLobbyGameMode::ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId)
{
	if (!LobbyItemManager) return;
	
	LobbyItemManager->ApplyLobbyItem(Buyer, ItemId);
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

	if (!PTWGameState) return;

	PTWGameState->UpdateLobbyRankingDataMap(PlayerState->GetUniqueId().ToString(), PlayerState->GetPlayerData());
}


