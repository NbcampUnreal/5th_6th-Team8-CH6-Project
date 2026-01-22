// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"

void UPTWScoreSubsystem::SavePlayersData()
{
	
}

void UPTWScoreSubsystem::SaveCurrentGameRound(int32 NewGameRound)
{
	CurrentGameRound = NewGameRound;
}



// 1. 게임 스테이트에서 현재 미니 게임 점수 관리.
// 2. 미니 게임이 끝나고 로비로 이동하기 전 순위 계산하고 얻은 승리 포인트 및 골드 서브 시스템에 전달
// 3. 로비-> 미니 게임 이동 하기 전 서브 시스템에 데이터 전달
