// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWDummyBotController.h"
#include "Debug/PTWLogCategorys.h"
#include "Kismet/GameplayStatics.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWGameLiftServerSubsystem.h"
#include "System/PTWScoreSubsystem.h"
#include "System/PTWSteamSessionSubsystem.h"


APTWGameMode::APTWGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		CachedGameData = PTWScoreSubsystem->GetSavedGameData();
		AllPlayer = PTWScoreSubsystem->GetServerTravelPlayerCount();
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

				ApplyPlayerGameDataFromSubsystem(NewBotCon);
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

#if WITH_GAMELIFT
	if (!FParse::Param(FCommandLine::Get(), *PTWSessionKey::NoGameLift.ToString()))
	{
		UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this);
		GameLiftServerSubsystem->UpdatePlayerCount(TEXT("Join"));
	}
#endif
	APlayerState* PS = NewPlayer->PlayerState;
	FString UniqueId = IsValid(PS) && PS->GetUniqueId().IsValid() ? PS->GetUniqueId().ToString() : TEXT("");
	
	if (UPTWGameInstance * GI = GetGameInstance<UPTWGameInstance>())
	{
		if (!UniqueId.IsEmpty() && !PS->GetPlayerName().IsEmpty())
		{
			GI->HandlePlayerUniqueIdReplicated(PS, UniqueId);
			GI->HandlePlayerNameReplicated(PS, PS->GetPlayerName());
		}
	}
	
	HandlePlayerJoined(NewPlayer);
}

void APTWGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	ApplyPlayerGameDataFromSubsystem(NewPlayer);
	
}

void APTWGameMode::Logout(AController* Exiting)
{
	FString PlayerName = TEXT("Unknown");
	FUniqueNetIdRepl CachedUniqueNetId;
	FString CachedPlayerSessionId;
	
	if (APlayerState* PS = Exiting->GetPlayerState<APlayerState>())
	{
		PlayerName = PS->GetPlayerName();
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(Exiting))
		{
			CachedPlayerSessionId = PC->GetPlayerSessionId();
		}
	}
	
	Super::Logout(Exiting);

#if WITH_GAMELIFT
	UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this);
	if (!FParse::Param(FCommandLine::Get(), *PTWSessionKey::NoGameLift.ToString()))
	{
		if (IsValid(GameLiftServerSubsystem) && !CachedPlayerSessionId.IsEmpty())
		{
			// 로그아웃한 PlayerSession 제거
			GameLiftServerSubsystem->RemovePlayerSession(CachedPlayerSessionId);
			// 플레이어 수 감소
			GameLiftServerSubsystem->UpdatePlayerCount(TEXT("Leave"));
			if (GetNumPlayers() <= 0)
			{
				FTimerHandle ExitTempTimerHandle;
				GetWorldTimerManager().SetTimer(ExitTempTimerHandle, [=, this]()
				{
					GameLiftServerSubsystem->OnUpdateSessionStateCompleted.Execute(TEXT("TERMINATE"));
				}, 30.0f, false);
				GameLiftServerSubsystem->OnUpdateSessionStateCompleted.BindLambda([GameLiftServerSubsystem](const FString& Action)
				{
					if(Action == TEXT("TERMINATE"))
					{
						GameLiftServerSubsystem->ExitGameSession();
						FGenericPlatformMisc::RequestExit(false);
					}
				});
				
				GetWorld()->GetTimerManager().ClearTimer(GameLiftServerSubsystem->UpdateSessionStateTimer);
				GameLiftServerSubsystem->UpdateSessionState(TEXT("TERMINATE"));
				return;
			}
		}
	}
#endif
	
	if (!IsValid(PTWGameState)) return;

	UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>();
	if (!PTWScoreSubsystem) return;
	
	// 플레이어가 로그 아웃 했을 때 ready 상태였을 경우 ReadyPlayer 감소 
	if (APTWPlayerState* PlayerState = Exiting->GetPlayerState<APTWPlayerState>())
	{
		if (PlayerState->bIsReadyToPlay)
		{
			PlayerState->bIsReadyToPlay = false;
			ReadyPlayer = FMath::Max(0, ReadyPlayer - 1);
		}
		
		SavePlayerGameData(PTWScoreSubsystem, PlayerState);
	}
	
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

FString APTWGameMode::InitNewPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayer, UniqueId, Options, Portal);

	if (ErrorMessage.IsEmpty())
	{
		FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
		
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(NewPlayer))
		{
			PC->SetPlayerSessionId(PlayerSessionId);
			UE_LOG(LogTemp, Log, TEXT("PlayerSessionId [%s] 캐싱 완료!"), *PlayerSessionId);
		}
	}

	return ErrorMessage;
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
	if (!Controller) return;

	APTWPlayerState* PlayerState = Controller->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	if (!PlayerState->bIsReadyToPlay)
	{
		PlayerState->bIsReadyToPlay = true;
		ReadyPlayer++;
	}
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
	
}

void APTWGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	// 1. 함수 진입 확인
	UE_LOG(LogTemp, Warning, TEXT("======================================="));
	UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] HandleSeamlessTravelPlayer 호출됨! Controller: %s"), 
		C ? *C->GetName() : TEXT("NULL"));

	APlayerController* PC = Cast<APlayerController>(C);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelPlayer] 전달된 Controller가 PlayerController가 아닙니다."));
		Super::HandleSeamlessTravelPlayer(C);
		return;
	}

	// 2. PlayerState 유효성 및 상태 해제 확인
	if (PC->PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("[TravelPlayer] %s: PlayerState 확인. 관전 플래그 해제 시도."), 
			*PC->PlayerState->GetPlayerName());
		PC->PlayerState->SetIsSpectator(false);
		PC->PlayerState->SetIsOnlyASpectator(false);
		PC->bPlayerIsWaiting = false;
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TravelPlayer] 치명적 에러: %s의 PlayerState가 Null입니다! 관전 해제 불가!"), 
			*PC->GetName());
	}

	UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] Super 호출 전 현재 State: %s"), 
		*PC->GetStateName().ToString());

	// 3. 엔진 기본 트래블 로직 실행
	Super::HandleSeamlessTravelPlayer(C);

	UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] Super 호출 후 State: %s, 현재 Pawn: %s"), 
		*PC->GetStateName().ToString(), PC->GetPawn() ? *PC->GetPawn()->GetName() : TEXT("None"));

	// 4. 관전자 상태 변경 로직 검증
	const FName CurrentState = PC->GetStateName();
	if (CurrentState == NAME_Spectating || CurrentState == NAME_Inactive)
	{
		PC->ChangeState(NAME_Playing);
		PC->ClientGotoState(NAME_Playing);

		UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] %s 상태를 %s에서 Playing으로 강제 전환 및 클라이언트 동기화 완료"), 
			PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName(), *CurrentState.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[TravelPlayer] %s는 이미 %s 상태입니다. 상태 전환 건너뜀."), 
			PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName(), *CurrentState.ToString());
	}

	// 5. 폰 제거 로직 추적
	if (APawn* CurrentPawn = PC->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] %s의 기존 폰(%s) 파괴 시도"), 
			PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName(), *CurrentPawn->GetName());
		CurrentPawn->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer] %s는 파괴할 기존 폰이 없습니다."), 
			PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName());
	}

	// 6. 다음 틱(NextTick) 재스폰 람다 추적
	GetWorld()->GetTimerManager().SetTimerForNextTick([PC, this]()
		{
			if (!PC)
			{
				UE_LOG(LogTemp, Error, TEXT("[TravelPlayer_NextTick] PC가 날아가서 재스폰이 취소되었습니다!"));
				return;
			}

			if (!PC->GetPawn())
			{
				UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer_NextTick] %s 폰 재스폰(RestartPlayer) 시도..."), 
					PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName());

				RestartPlayer(PC);

				if (PC->GetPawn())
				{
					PC->SetViewTarget(PC);
					UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer_NextTick] %s 새 Pawn(%s) 재스폰 및 뷰타겟 세팅 완벽 성공!"), 
						PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName(), *PC->GetPawn()->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[TravelPlayer_NextTick] 실패! %s RestartPlayer 호출 후에도 폰이 없습니다! (스폰 포인트가 없거나 게임모드 로직 문제)"), 
						PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[TravelPlayer_NextTick] %s는 이미 폰(%s)을 가지고 있어 재스폰을 건너뜁니다."), 
					PC->PlayerState ? *PC->PlayerState->GetPlayerName() : *PC->GetName(), *PC->GetPawn()->GetName());
			}
		});
}

void APTWGameMode::SetTravelLevelName(const FString& InLevelName)
{ 
	TravelLevelName = InLevelName; 
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
	UE_LOG(Log_GameMode, Log, TEXT("Clearing Timer") );
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

void APTWGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	
#if WITH_GAMELIFT
	if (!FParse::Param(FCommandLine::Get(), *PTWSessionKey::NoGameLift.ToString()))
	{
		FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
		if (!PlayerSessionId.IsEmpty())
		{
			UPTWGameLiftServerSubsystem* GameLiftServerSubsystem = UPTWGameLiftServerSubsystem::Get(this);
			if (IsValid(GameLiftServerSubsystem))
			{
				bool bIsSuccess = GameLiftServerSubsystem->AcceptPlayerSession(PlayerSessionId);
				if (!bIsSuccess)
				{
					ErrorMessage = TEXT("Invalid GameLift Player Session");
				}
			}
		}
		else
		{
			ErrorMessage = TEXT("Missing PlayerSessionId");
		}
	}
#endif
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

void APTWGameMode::SavePlayerGameData(UPTWScoreSubsystem* ScoreSubsystem, APTWPlayerState* PlayerState)
{
	if (!IsValid(PlayerState) || !IsValid(ScoreSubsystem)) return;

	FPTWPlayerGameData PlayerGameData;
	PlayerGameData.PlayerData = PlayerState->GetPlayerData();
	PlayerGameData.LobbyItemData = PlayerState->GetLobbyItemData();
				
	ScoreSubsystem->SavePlayerGameData(PlayerState->GetUniqueId().ToString(),PlayerGameData);
}

void APTWGameMode::SaveGameDataToSubsystem()
{
	if (!PTWGameState) return;
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->SaveServerTravelPlayerCount(GetNumPlayers());
		PTWScoreSubsystem->SaveGameData(PTWGameState->GameData);

		for (APlayerState* PlayerState : PTWGameState->PlayerArray)
		{
			if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
			{
				SavePlayerGameData(PTWScoreSubsystem, PTWPlayerState);
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

void APTWGameMode::ApplyPlayerGameDataFromSubsystem(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
		{
			// if (FPTWPlayerData* FoundData = PTWScoreSubsystem->FindPlayerData(PTWPlayerState->GetPlayerName()))
			// {
			// 	PTWPlayerState->SetPlayerData(*FoundData);
			// 	
			// }
			// if (FPTWLobbyItemData* FoundData = PTWScoreSubsystem->FindLobbyItemData(PTWPlayerState->GetPlayerName()))
			// {
			// 	PTWPlayerState->SetLobbyItemData(*FoundData);
			// }

			if (FPTWPlayerGameData* FoundData = PTWScoreSubsystem->FindPlayerGameData(PTWPlayerState->GetUniqueId().ToString()))
			{
				PTWPlayerState->SetPlayerData(FoundData->PlayerData);
				PTWPlayerState->SetLobbyItemData(FoundData->LobbyItemData);
			}
			else
			{
				PTWScoreSubsystem->AddConnectedPlayerId(PTWPlayerState->GetUniqueId().ToString());
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






