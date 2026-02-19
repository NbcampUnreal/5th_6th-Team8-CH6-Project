// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
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
		AllPlayer = PTWScoreSubsystem->GetSavedPlayerCount();
	}
}

void APTWGameMode::InitGameState()
{
	Super::InitGameState();

	PTWGameState = GetGameState<APTWGameState>();

	if (PTWGameState)
	{
		PTWGameState->SetCurrentRound(CurrentRound); // GameMode 라운드 값 받아서 GameState에 전달
	}
}

void APTWGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (PTWGameState)
	{
		PTWGameState->OnTimerFinished.AddDynamic(this, &APTWGameMode::EndTimer);
	}
}

void APTWGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->IncreasePlayerCount();
	}

	FString PlayerName = TEXT("Unknown");
	if (APTWPlayerState* PS = NewPlayer->GetPlayerState<APTWPlayerState>())
	{
		PlayerName = PS->GetPlayerName();
	}
	if (PTWGameState)
	{
		FString JoinMsg = FString::Printf(TEXT("Player '%s' has joined the game."), *PlayerName);
		PTWGameState->Multicast_SystemMessage(JoinMsg);
	}
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
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->DecreasePlayerCount();
		AllPlayer = PTWScoreSubsystem->GetSavedPlayerCount();
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
}

void APTWGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

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

void APTWGameMode::PlayerReadyToPlay(APlayerController* ReadyPlayerController)
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
		PTWScoreSubsystem->SavePlayerCount(PTWGameState->PlayerArray.Num());

		for (APlayerState* PlayerState : PTWGameState->PlayerArray)
		{
			if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
			{
				PTWScoreSubsystem->SavePlayerData(PTWPlayerState->GetPlayerName(), PTWPlayerState->GetPlayerData());
			}
		}
	}
}

void APTWGameMode::ApplyPlayerDataFromSubsystem(APlayerController* NewPlayer)
{
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
		{
			if (FPTWPlayerData* FoundData = PTWScoreSubsystem->FindPlayerData(PTWPlayerState->GetPlayerName()))
			{
				PTWPlayerState->SetPlayerData(*FoundData);

				UE_LOG(LogTemp, Warning, TEXT("Player Gold: %d"), FoundData->Gold);
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






