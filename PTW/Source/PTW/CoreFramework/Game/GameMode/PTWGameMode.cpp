// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWDummyBotController.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"


APTWGameMode::APTWGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		CurrentRound = PTWScoreSubsystem->GetCurrentGameRound(); // GameInstance 라운드 값 받아서 GameMode에 저장
		CachedGameData = PTWScoreSubsystem->GetSavedGameData();
		AllPlayer = PTWScoreSubsystem->GetSavedAllPlayerCount();
	}
}

void APTWGameMode::InitGameState()
{
	Super::InitGameState();

	PTWGameState = GetGameState<APTWGameState>();

	if (PTWGameState)
	{
		PTWGameState->SetCurrentRound(CurrentRound); // GameMode 라운드 값 받아서 GameState에 전달
		PTWGameState->GameData = CachedGameData;
	}
}

void APTWGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (PTWGameState)
	{
		PTWGameState->OnTimerFinished.AddDynamic(this, &APTWGameMode::EndTimer);
	}

	if (UPTWScoreSubsystem* ScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BotTravel] 새 맵 도착! 복구해야 할 봇 수: %d"), ScoreSubsystem->TravelingBotNames.Num());

		for (const FString& BotName : ScoreSubsystem->TravelingBotNames)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BotTravel] %s 봇 복구(스폰) 시도 중..."), *BotName);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APTWDummyBotController* NewBotCon = GetWorld()->SpawnActor<APTWDummyBotController>(
				APTWDummyBotController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (NewBotCon)
			{
				UE_LOG(LogTemp, Warning, TEXT("[BotTravel] -> %s AIController 스폰 성공!"), *BotName);

				if (APlayerState* PS = NewBotCon->GetPlayerState<APlayerState>())
				{
					PS->SetPlayerName(BotName);
				}

				RestartPlayer(NewBotCon);

				if (NewBotCon->GetPawn())
				{
					UE_LOG(LogTemp, Warning, TEXT("[BotTravel] -> %s 캐릭터 폰(Pawn) 빙의 완전 성공! 위치: %s"), *BotName, *NewBotCon->GetPawn()->GetActorLocation().ToString());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[BotTravel] -> [에러] %s 봇이 RestartPlayer를 호출했지만 Pawn을 받지 못했습니다! (투명 봇 상태)"), *BotName);
				}

				ApplyPlayerDataFromSubsystem(NewBotCon);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[BotTravel] -> [에러] %s 봇의 AIController 스폰 자체가 실패했습니다!"), *BotName);
			}
		}

		ScoreSubsystem->TravelingBotNames.Empty();
	}
}

void APTWGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	HandlePlayerJoined(NewPlayer);
}

void APTWGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	ApplyPlayerDataFromSubsystem(NewPlayer);
	
}

void APTWGameMode::Logout(AController* Exiting)
{
	FString PlayerName = TEXT("Unknown");
	if (APlayerState* PS = Exiting->GetPlayerState<APlayerState>())
	{
		PlayerName = PS->GetPlayerName();
	}
	
	Super::Logout(Exiting);

	if (!IsValid(PTWGameState)) return;

	// 플레이어가 로그 아웃 했을 때 ready 상태였을 경우 ReadyPlayer 감소 
	if (APTWPlayerState* PlayerState = Exiting->GetPlayerState<APTWPlayerState>())
	{
		if (PlayerState->bIsReadyToPlay)
		{
			PlayerState->bIsReadyToPlay = false;
			ReadyPlayer = FMath::Max(0, ReadyPlayer - 1);
		}
	}
	
	AllPlayer--;
	
	if (PTWGameState)
	{
		FString LeaveMsg = FString::Printf(TEXT("Player '%s' has left the game."), *PlayerName);
		PTWGameState->Multicast_SystemMessage(LeaveMsg);
	}
	
	CheckAllPlayersLoaded();
}

void APTWGameMode::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (APTWDummyBotController* BotCon = Cast<APTWDummyBotController>(It->Get()))
		{
			ActorList.Add(BotCon);

			if (BotCon->PlayerState)
			{
				ActorList.Add(BotCon->PlayerState);
			}
		}
	}
}

void APTWGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	ReadyPlayer = 0;
	AllPlayer = GetNumPlayers();

	UE_LOG(LogTemp, Warning, TEXT("PostSeamlessTravel AllPlayer: %d"), AllPlayer);
	
	// 모든 사람 플레이어 준비 상태 초기화
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerState* PlayerState = (*It)->GetPlayerState<APTWPlayerState>())
		{
			PlayerState->bIsReadyToPlay = false;
		}
	}

	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->LoadedPlayerCount = 0;
		GS->TotalPlayerCount = GetNumPlayers();
	}
}

void APTWGameMode::PrepareAllPlayersLoadingScreen(ELoadingScreenType Type, FName MapRowName)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(*It))
		{
			PC->Client_PrepareLoadingScreen(Type, MapRowName);
		}
	}
}

void APTWGameMode::CheckAllPlayersLoaded()
{
	APTWGameState* GS = GetGameState<APTWGameState>();
	// 일정 인원 이상 혹은 전체 인원 체크
	if (GS && GS->LoadedPlayerCount >= GS->TotalPlayerCount)
	{
		Multicast_CloseLoadingScreen();
	}
}

void APTWGameMode::PlayerReadyToPlay(APlayerController* Controller)
{
	ReadyPlayer++;
}

void APTWGameMode::Multicast_CloseLoadingScreen_Implementation()
{
	if (UPTWGameInstance* GI = Cast<UPTWGameInstance>(GetGameInstance()))
	{
		GI->StopLoadingScreen();
	}
}

void APTWGameMode::HandlePlayerJoined(AController* JoinedController)
{
	if (!JoinedController) return;

	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->IncreasePlayerCount();
	}

	FString PlayerName = TEXT("Unknown");
	if (APTWPlayerState* PS = JoinedController->GetPlayerState<APTWPlayerState>())
	{
		PlayerName = PS->GetPlayerName();
	}

	if (PTWGameState)
	{
		FString JoinMsg = FString::Printf(TEXT("Player '%s' has joined the game."), *PlayerName);
		PTWGameState->Multicast_SystemMessage(JoinMsg);
	}

	AllPlayer = GetNumPlayers();
}

void APTWGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	if (APlayerController* PC = Cast<APlayerController>(C))
	{
		if (PC->PlayerState)
		{
			PC->PlayerState->SetIsSpectator(false);
			PC->PlayerState->SetIsOnlyASpectator(false);
			PC->bPlayerIsWaiting = false;
		}
	}

	Super::HandleSeamlessTravelPlayer(C);

	if (APlayerController* PC = Cast<APlayerController>(C))
	{
		const FName CurrentState = PC->GetStateName();
		if (CurrentState == NAME_Spectating || CurrentState == NAME_Inactive)
		{
			PC->ChangeState(NAME_Playing);
			UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] %s 상태를 %s에서 Playing으로 전환"), *PC->PlayerState->GetPlayerName(), *CurrentState.ToString());
		}

		if (APawn* CurrentPawn = PC->GetPawn())
		{
			CurrentPawn->Destroy();
		}

		GetWorld()->GetTimerManager().SetTimerForNextTick([PC, this]()
			{
				if (PC && !PC->GetPawn())
				{
					RestartPlayer(PC);
					PC->SetViewTarget(PC);
					UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] %s 플레이어 Pawn 재스폰 완료"), *PC->PlayerState->GetPlayerName());
				}
			});
	}
}

void APTWGameMode::StartTimer(float TimeDuration)
{
	if (PTWGameState)
	{
		PTWGameState->SetRemainTime(TimeDuration);
	}
	
	GetWorldTimerManager().SetTimer(TimerHandle, this, &APTWGameMode::UpdateTimer, 1.f, true, 1.f);
}

void APTWGameMode::ClearTimer()
{
	GetWorldTimerManager().ClearTimer(TimerHandle);
}

void APTWGameMode::EndTimer()
{
	ClearTimer();
	
	if (PTWGameState->GetCurrentGamePhase() != EPTWGamePhase::MiniGame)
	{
		// 잠깐 딜레이 후 ServerTravel 실행
		FTimerHandle DelayTimerHandle;
		GetWorldTimerManager().SetTimer(DelayTimerHandle, this, &APTWGameMode::TravelLevel, 2.f);
	}
}

void APTWGameMode::TravelLevel()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(*It))
		{
			PC->Client_DisplayLoadingScreen();
		}
	}
	SaveGameDataToSubsystem();
	GetWorld()->ServerTravel(TravelLevelName);
}

void APTWGameMode::MovePlayerToStart(AController* Controller)
{
	if (!Controller) return;

	AActor* PlayerStart = ChoosePlayerStart(Controller);
	if (!PlayerStart) return;

	APawn* Pawn= Controller->GetPawn();
	if (!Pawn) return;

	Pawn->SetActorLocation(PlayerStart->GetActorLocation());
	Pawn->SetActorRotation(PlayerStart->GetActorRotation());
}

void APTWGameMode::SetInputBlock(bool bInputBlock)
{
	//PlayerController->Client_SetInputRestricted(bInputBlock);
	
	if (!PTWGameState) return;
	
	PTWGameState->bGlobalInputBlocked = bInputBlock;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			if (PC->IsLocalController())
			{
				PC->ApplyInputRestricted(bInputBlock);
			}
		}
	}
}

void APTWGameMode::SaveGameDataToSubsystem()
{
	if (!PTWGameState) return;
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->SaveGameRound(PTWGameState->GetCurrentRound());
		PTWScoreSubsystem->SaveAllPlayerCount(GetNumPlayers());
		PTWScoreSubsystem->SaveGameData(PTWGameState->GameData);

		for (APlayerState* PlayerState : PTWGameState->PlayerArray)
		{
			if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
			{
				PTWScoreSubsystem->SavePlayerData(PTWPlayerState->GetPlayerName(), PTWPlayerState->GetPlayerData());
				PTWScoreSubsystem->SaveLobbyItemData(PTWPlayerState->GetPlayerName(), PTWPlayerState->GetLobbyItemData());
			}
		}

		PTWScoreSubsystem->TravelingBotNames.Empty();
		int32 SavedBotCount = 0;

		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			if (APTWDummyBotController* BotCon = Cast<APTWDummyBotController>(It->Get()))
			{
				if (APlayerState* PS = BotCon->GetPlayerState<APlayerState>())
				{
					FString BName = PS->GetPlayerName();
					PTWScoreSubsystem->TravelingBotNames.Add(BName);
					SavedBotCount++;
					UE_LOG(LogTemp, Warning, TEXT("[BotTravel] 로비에서 봇 명단 저장됨: %s"), *BName);
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[BotTravel] 트래블 직전 총 %d 마리의 봇 명단이 서브시스템에 기록됨!"), SavedBotCount);
	}
}

void APTWGameMode::ApplyPlayerDataFromSubsystem(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
		{
			if (FPTWPlayerData* FoundData = PTWScoreSubsystem->FindPlayerData(PTWPlayerState->GetPlayerName()))
			{
				PTWPlayerState->SetPlayerData(*FoundData);

				UE_LOG(LogTemp, Warning, TEXT("Player Gold: %d"), FoundData->Gold);
			}
			if (FPTWLobbyItemData* FoundData = PTWScoreSubsystem->FindLobbyItemData(PTWPlayerState->GetPlayerName()))
			{
				PTWPlayerState->SetLobbyItemData(*FoundData);
			}
		}
	}
}

void APTWGameMode::UpdateTimer()
{
	if (PTWGameState)
	{
		PTWGameState->DecreaseTimer();
	}
}






