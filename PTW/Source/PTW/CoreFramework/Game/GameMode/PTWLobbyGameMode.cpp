// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"


void APTWLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	if (UPTWGameInstance* PTWGameInstance = Cast<UPTWGameInstance>(GetGameInstance()))
	{
		if (PTWGameInstance->bIsFirstLobby == true)
		{
			bIsFirstLobby = true;	
			PTWGameInstance->bIsFirstLobby = false;
		}
		else
		{
			bIsFirstLobby = false;	
		}
	}
}

void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");

	if (IsValid(PTWGameState))
	{
		if (bIsFirstLobby == true)
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::PreGameLobby);
		}
		else
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);
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
		AddRandomGold(NewPlayer);
		
		// PreGameLobby 상태에서 최소 인원 충족 되면 WaitingTimer 시작
		// if (PTWGameState->PlayerArray.Num() >= GameFlowRule.MinPlayersToStart)
		// {
		// 	if (bWaitingTimerStarted == false)
		// 	{
		// 		StartTimer(GameFlowRule.WaitingTime);
		// 		bWaitingTimerStarted = true;
		// 	}
		// }

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

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PostGameLobby)
	{
		// 로딩 UI 
		if (PTWGameState->PlayerArray.Num() == 1) // 임시 설정
		{
			// 플레이 중인 모든 플레이어 접속 중이면 로딩 UI 해제
		}

		if (!GetWorldTimerManager().IsTimerActive(TimerHandle))
		{
			StartTimer(GameFlowRule.NextMiniGameWaitTime);

			FTimerHandle RouletteDelayTimerHandle;
			GetWorldTimerManager().SetTimer(RouletteDelayTimerHandle, this, &APTWLobbyGameMode::StartRoulette, GameFlowRule.RouletteDelay);
		}
	}
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (!IsValid(PTWGameState)) return;

	// if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	// {
	// 	if (PTWGameState->PlayerArray.Num() < GameFlowRule.MinPlayersToStart)
	// 	{
	// 		if (bWaitingTimerStarted == true)
	// 		{
	// 			ClearTimer();
	// 			bWaitingTimerStarted = false;
	// 		}
	// 	}
	// }
}

void APTWLobbyGameMode::AddRandomGold(APlayerController* NewPlayer)
{
	int32 RandomGold = FMath::RandRange(1, 100);

	if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
		PlayerData.Gold = RandomGold;
		PTWPlayerState->SetPlayerData(PlayerData);

		UE_LOG(LogTemp, Warning, TEXT("RandomGold: %d"), RandomGold);
	}
}

void APTWLobbyGameMode::StartRoulette()
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
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->DisplayName.ToString());
	}
	
	PTWGameState->SetSelectedMapRowName(SelectedRowName); // GameState에 전달
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


