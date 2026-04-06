// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Component/PTWWinConditionComponent.h"

#include "CoreFramework/PTWPlayerRoundDataInterface.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/Interface/PTWGameModeInterface.h"
#include "GameFramework/PlayerState.h"


UPTWWinConditionComponent::UPTWWinConditionComponent()
{
}

void UPTWWinConditionComponent::InitWinConditionComponent(APTWGameState* InGameState,
                                                          const FPTWMiniGameRule* InMiniGameRule)
{
	this->GameState = InGameState;
	this->MiniGameRule = InMiniGameRule;
}

void UPTWWinConditionComponent::CheckEndGameCondition()
{
	switch (MiniGameRule->WinConditionRule.WinType)
	{
	case EPTWWinType::Survival:
		CheckSurvivalCondition();
		break;
	case EPTWWinType::Target:
		CheckTargetScoreCondition();
		break;
	default:
		break;
	}
}

void UPTWWinConditionComponent::CheckSurvivalCondition()
{
	if (!GameState) return;

	IPTWGameModeInterface* GameModeInterface = Cast<IPTWGameModeInterface>(GetOwner());
	if (!GameModeInterface) return;
	
	if (MiniGameRule->TeamRule.bUseTeam)
	{
		TSet<int32> AliveTeams;
		for (APlayerState* Player : GameState->AlivePlayers)
		{
			if (IPTWPlayerRoundDataInterface* PlayerRoundDataInterface = Cast<IPTWPlayerRoundDataInterface>(Player))
			{
				AliveTeams.Add(PlayerRoundDataInterface->GetTeamId());
			}
		}
		
		if (AliveTeams.Num() == 1)
		{
			GameState->SetWinTeamId(*AliveTeams.begin());
			GameModeInterface->EndGame();
		}
		else if (AliveTeams.IsEmpty())
		{
			// 생존 팀이 없을 경우 가장 마지막에 죽은 플레이어의 팀 승리
			if (IPTWPlayerRoundDataInterface* Last = FindLastDeadPlayer())
			{
				GameState->SetWinTeamId(Last->GetTeamId());
				GameModeInterface->EndGame();
			}
		}
	}
	else
	{
		if (GameState->AlivePlayers.Num() == 1)
		{
			GameModeInterface->EndGame();
		}
		else if (GameState->AlivePlayers.IsEmpty())
		{
			if (IPTWPlayerRoundDataInterface* Last = FindLastDeadPlayer())
			{
				GameState->AlivePlayers.Add(Cast<APlayerState>(Last));
				GameModeInterface->EndGame();
			}
		}
	}
}

void UPTWWinConditionComponent::CheckTargetScoreCondition()
{
	if (!GameState) return;

	IPTWGameModeInterface* GameModeInterface = Cast<IPTWGameModeInterface>(GetOwner());
	if (!GameModeInterface) return;
	
	if (MiniGameRule->TeamRule.bUseTeam && MiniGameRule->TeamRule.bShareScoreWithinTeam)
	{
		for (const FPTWTeamInfo& TeamInfo : GameState->GetTeams())
		{
			if (TeamInfo.TeamScore >= MiniGameRule->WinConditionRule.TargetValue)
			{
				GameState->SetWinTeamId(TeamInfo.TeamID);
				GameModeInterface->EndGame();
				return;
			}
		}
		
	}
	else
	{
		TArray<APTWPlayerState*> RankedPlayers = GameState->GetRankedPlayers();
		if (RankedPlayers.IsEmpty()) return;

		if (RankedPlayers[0]->GetPlayerRoundData().Score >= MiniGameRule->WinConditionRule.TargetValue)
		{
			if (MiniGameRule->TeamRule.bUseTeam && !MiniGameRule->TeamRule.bShareScoreWithinTeam)
			{
				GameState->SetWinTeamId(RankedPlayers[0]->GetTeamId());
			}
			GameModeInterface->EndGame();
		}
	}
}

IPTWPlayerRoundDataInterface* UPTWWinConditionComponent::FindLastDeadPlayer()
{
	if (!GameState) return nullptr;
	
	IPTWPlayerRoundDataInterface* LastInterface = nullptr;
	int32 HighDeathOrder = -1;

	for (APlayerState* Player : GameState->PlayerArray)
	{
		if (IPTWPlayerRoundDataInterface* Interface = Cast<IPTWPlayerRoundDataInterface>(Player))
		{
			if (Interface->GetDeathOrder() > HighDeathOrder)
			{
				HighDeathOrder = Interface->GetDeathOrder();
				LastInterface = Interface;
			}
		}
	}
	return LastInterface;
}


