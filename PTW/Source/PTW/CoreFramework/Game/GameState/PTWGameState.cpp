// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameState.h"

#include "CoreFramework/PTWPlayerState.h"
#include "Net/UnrealNetwork.h"

APTWGameState::APTWGameState()
{
	bReplicates = true;
}


void APTWGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWGameState, RemainTime);
	DOREPLIFETIME(APTWGameState, CurrentRound);
	DOREPLIFETIME(APTWGameState, CurrentGamePhase);
	DOREPLIFETIME(APTWGameState, RankedPlayers);
}

void APTWGameState::UpdateRanking()
{
	//TArray<APTWPlayerState*> RankingPlayers;
	RankedPlayers.Reset();
	
	for (APlayerState* PlayerState:PlayerArray)
	{
		if(APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
		{
			if (IsValid(PTWPlayerState))
			{
				RankedPlayers.Add(PTWPlayerState);
			}
		}
	}
	
	RankedPlayers.Sort([](const APTWPlayerState& A, const APTWPlayerState& B) {
       
		   if (!IsValid(&A)) return false;     
		   if (!IsValid(&B)) return true;

		   const auto& APD = A.GetPlayerRoundData();
		   const auto& BPD = B.GetPlayerRoundData();
       
		   return APD.Score > BPD.Score;
	});
}

void APTWGameState::AddRankedPlayer(APTWPlayerState* NewPlayerState)
{
	RankedPlayers.AddUnique(NewPlayerState);
}

void APTWGameState::ApplyMiniGameRankScore(const FPTWMiniGameRule& MiniGameRule)
{
	// 현재 랭킹을 기준으로 승리 포인트 추가
	
	// 승리 포인트는 임시로 플레이어 인원 수만큼 지급
	// 동점 계산 X
	for (int i = 0; i < RankedPlayers.Num(); i++)
	{
		FPTWPlayerData PlayerData = RankedPlayers[i]->GetPlayerData();
		PlayerData.TotalWinPoints += RankedPlayers.Num() - i;
		RankedPlayers[i]->SetPlayerData(PlayerData);
	}
}




void APTWGameState::DecreaseTimer()
{
	if (!HasAuthority()) return;
	
	if (RemainTime <= 0)
	{
		OnTimerFinished.Broadcast();
	}
	else
	{
		RemainTime--;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRemainTimeChanged.Broadcast(RemainTime);
		}
	}
}

void APTWGameState::AdvanceRound()
{
	CurrentRound++;
	UE_LOG(LogTemp, Warning, TEXT("Current Round: %d"), CurrentRound);
}

void APTWGameState::SetRemainTime(int32 NewTime)
{
	if (!HasAuthority()) return;

	RemainTime = NewTime;
}

void APTWGameState::SetCurrentRound(int32 NewRound)
{
	if (!HasAuthority()) return;
	
	CurrentRound = NewRound;
}

void APTWGameState::SetCurrentPhase(EPTWGamePhase NewGamePhase)
{
	if (!HasAuthority()) return;

	CurrentGamePhase = NewGamePhase;
}

void APTWGameState::SetSelectedMapRowName(FName MapRowName)
{
	if (!HasAuthority()) return;

	SelectedMapRowName = MapRowName;

	OnRep_SelectedMapRowName();
}

void APTWGameState::Multicast_BroadcastKilllog_Implementation(AActor* DeadActor, AActor* KillerActor)
{
	if (OnKilllogBroadcast.IsBound())
	{
		OnKilllogBroadcast.Broadcast(DeadActor, KillerActor);
	}
}

void APTWGameState::OnRep_RemainTime()
{
	OnRemainTimeChanged.Broadcast(RemainTime);
}

void APTWGameState::OnRep_CurrentRound()
{
	OnRoundChanged.Broadcast(CurrentRound);
}

void APTWGameState::OnRep_CurrentGamePhase()
{
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}

void APTWGameState::OnRep_RankedPlayers()
{
	OnUpdateRankedPlayers.Broadcast(RankedPlayers);
}

void APTWGameState::OnRep_SelectedMapRowName()
{
	OnSelectedMiniGameMap.Broadcast(SelectedMapRowName);
}
