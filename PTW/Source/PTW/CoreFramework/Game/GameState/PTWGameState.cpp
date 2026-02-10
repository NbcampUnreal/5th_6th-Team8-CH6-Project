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
	DOREPLIFETIME(APTWGameState, RouletteData);
	DOREPLIFETIME(APTWGameState, PortalCurrent);
	DOREPLIFETIME(APTWGameState, PortalRequired);
	DOREPLIFETIME(APTWGameState, bMiniGameCountdown);
	DOREPLIFETIME(APTWGameState, MiniGameCountDown);
	DOREPLIFETIME(APTWGameState, CurrentMiniGameRound);
	DOREPLIFETIME(APTWGameState, MaxMiniGameRound);

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

void APTWGameState::DecreaseCoundDown()
{
	if (!HasAuthority()) return;

	if (MiniGameCountDown <= 0)
	{
		OnCountDownFinished.Broadcast();
	}
	else
	{
		MiniGameCountDown--;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnMiniGameCountdownValueChanged.Broadcast(MiniGameCountDown);
		}
	}
}

void APTWGameState::AdvanceRound()
{
	CurrentRound++;
	UE_LOG(LogTemp, Warning, TEXT("Current Round: %d"), CurrentRound);
}

void APTWGameState::AdvanceMiniGameRound()
{
	CurrentMiniGameRound++;
}

void APTWGameState::SetRemainTime(int32 NewTime)
{
	if (!HasAuthority()) return;

	RemainTime = NewTime;
	
	OnRep_RemainTime();
}

void APTWGameState::SetCurrentRound(int32 NewRound)
{
	if (!HasAuthority()) return;
	
	CurrentRound = NewRound;
	
	OnRep_CurrentRound();
}

void APTWGameState::SetCurrentPhase(EPTWGamePhase NewGamePhase)
{
	if (!HasAuthority()) return;

	CurrentGamePhase = NewGamePhase;

	OnRep_CurrentGamePhase();
}

void APTWGameState::SetRouletteData(const FPTWRouletteData& NewData)
{
	if (!HasAuthority()) return;

	RouletteData = NewData;
	
	OnRep_RouletteData();
}

void APTWGameState::SetPortalCount(int32 NewCurrent, int32 NewRequired)
{
	if (!HasAuthority()) return;

	PortalCurrent = NewCurrent;
	PortalRequired = NewRequired;

	OnRep_PortalCount();
}

void APTWGameState::SetbMiniGameCountdown(bool bCountdown)
{
	if (!HasAuthority()) return;

	// 값이 실제로 변할 때만 처리 (네트워크 트래픽 최적화)
	if (bMiniGameCountdown == bCountdown) return;

	bMiniGameCountdown = bCountdown;
	OnRep_MiniGameCountdown();
}

void APTWGameState::SetMiniGameCountdown(int32 NewValue)
{
	if (!HasAuthority()) return;

	MiniGameCountDown = NewValue;
	OnRep_MiniGameCountDownValue();
}

void APTWGameState::SetMaxMiniGameRound(int32 NewMaxRound)
{
	if (!HasAuthority()) return;

	MaxMiniGameRound = NewMaxRound;
	OnRep_MaxMiniGameRound();
}

void APTWGameState::BroadcastChatMessage(const FString& Sender, const FString& Message)
{
	// 서버 전용
	if (!HasAuthority())
	{
		return;
	}

	Multicast_BroadcastChatMessage(Sender, Message);
}

void APTWGameState::Multicast_BroadcastKilllog_Implementation(AActor* DeadActor, AActor* KillerActor)
{
	if (OnKilllogBroadcast.IsBound())
	{
		OnKilllogBroadcast.Broadcast(DeadActor, KillerActor);
	}
}

void APTWGameState::Multicast_BroadcastKilllogEx_Implementation(AActor* DeadActor, AActor* KillerActor, FName CauseId)
{
	if (OnKilllogBroadcastEx.IsBound())
	{
		OnKilllogBroadcastEx.Broadcast(DeadActor, KillerActor, CauseId);
	}
}

void APTWGameState::Multicast_BroadcastChatMessage_Implementation(const FString& Sender, const FString& Message)
{
	OnChatMessageBroadcast.Broadcast(Sender, Message);
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

void APTWGameState::OnRep_RouletteData()
{
	OnRoulettePhaseChanged.Broadcast(RouletteData);
}

void APTWGameState::OnRep_PortalCount()
{
	OnPortalCountChanged.Broadcast(PortalCurrent, PortalRequired);
}

void APTWGameState::OnRep_MiniGameCountDownValue()
{
	OnMiniGameCountdownValueChanged.Broadcast(MiniGameCountDown);
}

void APTWGameState::OnRep_MiniGameCountdown()
{
	OnMiniGameCountdownChanged.Broadcast(bMiniGameCountdown);
}
void APTWGameState::OnRep_CurrentMiniGameRound()
{
	OnMiniGameRoundChanged.Broadcast(CurrentMiniGameRound, MaxMiniGameRound);
}

void APTWGameState::OnRep_MaxMiniGameRound()
{
	
}
