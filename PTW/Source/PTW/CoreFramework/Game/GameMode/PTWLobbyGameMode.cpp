// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"

// void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
// {
// 	Super::PostLogin(NewPlayer);
//
// 	//접속하면 무적 상태로 변경
// 	
// 	if (PTWGameState->PlayerArray.Num() >= MinPlayersToStart)
// 	{
// 		// 대기 타이머 시작. 미니 게임 선택은 언제?
// 		// 라운드 저장해서 처음 로비인지 게임 진행 중인지 체크
// 		// 일단 첫 로비라고 가정하고 최소 인원 수 도달하면 타이머 작동
//
// 		FTimerHandle WaitingTimerHandle;
// 		GetWorldTimerManager().SetTimer(WaitingTimerHandle, this, &APTWLobbyGameMode::StartMiniGame, WaitingTime, false);
// 	}
// }

void APTWLobbyGameMode::StartMiniGame()
{
	
}
