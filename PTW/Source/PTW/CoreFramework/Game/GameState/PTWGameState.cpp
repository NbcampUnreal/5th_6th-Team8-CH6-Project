// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameState.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Debug/PTWLogCategorys.h"
#include "MiniGame/PTWMiniGameRule.h"
#include "System/Prop/PTWPropSubsystem.h"
#include "Net/UnrealNetwork.h"


APTWGameState::APTWGameState()
{
	bReplicates = true;
}


void APTWGameState::BeginPlay()
{
	Super::BeginPlay();

	
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
	DOREPLIFETIME(APTWGameState, Teams);
	DOREPLIFETIME(APTWGameState, PropSeed);
	DOREPLIFETIME(APTWGameState, PropData);

}

void APTWGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APTWGameState::UpdateRanking(const FPTWMiniGameRule& MiniGameRule)
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
	
	RankedPlayers.Sort([MiniGameRule](const APTWPlayerState& A, const APTWPlayerState& B) {
       
		if (!IsValid(&A)) return false;     
		if (!IsValid(&B)) return true;

		const auto& APD = A.GetPlayerRoundData();
		const auto& BPD = B.GetPlayerRoundData();
		
		if (MiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
		{
			// true면 생존 false면 사망
			const bool bAAlive = (APD.DeathOrder == 0);
			const bool bBAlive = (BPD.DeathOrder == 0);

			if (bAAlive != bBAlive)
			{
				return bAAlive;
			}

			if (!bAAlive) 
			{
				if (APD.DeathOrder != BPD.DeathOrder)
				{
					return APD.DeathOrder > BPD.DeathOrder;
				}
					
			}
			return APD.Score > BPD.Score;
		}
		else if (MiniGameRule.WinConditionRule.WinType == EPTWWinType::Target)
		{
			return APD.Score > BPD.Score;
		}
		else
		{
			return APD.Score > BPD.Score;
		}
	});
}

void APTWGameState::AddRankedPlayer(APTWPlayerState* NewPlayerState)
{
	RankedPlayers.AddUnique(NewPlayerState);
}

void APTWGameState::AddTeamScore(APlayerState* Player, int32 Score)
{
	if (IPTWPlayerDataInterface* RoundDataInterface = Cast<IPTWPlayerDataInterface>(Player))
	{
		int32 TeamId = RoundDataInterface->GetTeamId();
		Teams[TeamId].TeamScore += Score;
	}
}

void APTWGameState::ApplyMiniGameRankScore(const FPTWMiniGameRule& MiniGameRule)
{
	if (MiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
	{
		// 팀전 우선
		if (MiniGameRule.TeamRule.bUseTeam)
		{
			// 이긴 팀 전원에게 점수
			for (FPTWTeamInfo& Team : Teams)
			{
				if (Team.TeamID != WinTeamId) continue;
                
				for (APlayerState* Member : Team.Members)
				{
					APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(Member);
					if (!PTWPlayerState) continue;

					FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
					PlayerData.TotalWinPoints += MiniGameRule.ScoreRule.TotalScore;
					PTWPlayerState->SetPlayerData(PlayerData);
				}
			}
		}
		else
		{
			// 생존자에게만 점수
			if (AlivePlayers.Num() == 0) return;
            
			for (APlayerState* PlayerState : AlivePlayers)
			{
				APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
				if (!PTWPlayerState) continue;

				FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
				PlayerData.TotalWinPoints += MiniGameRule.ScoreRule.TotalScore;
				PTWPlayerState->SetPlayerData(PlayerData);
			}
		}
	}
	else
	{
		if (RankedPlayers.Num() == 0) return;
        
		for (int i = 0; i < RankedPlayers.Num(); i++)
		{
			FPTWPlayerData PlayerData = RankedPlayers[i]->GetPlayerData();
			PlayerData.TotalWinPoints += RankedPlayers.Num() - i;
			RankedPlayers[i]->SetPlayerData(PlayerData);
		}
	}
}

void APTWGameState::OnRep_LoadedPlayerCount()
{
	OnLoadingProgressChanged.Broadcast(LoadedPlayerCount);
}

void APTWGameState::OnRep_GlobalInputBlocked()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC);
	if (!PTWPC) return;

	PTWPC->ApplyInputRestricted(bGlobalInputBlocked);
}

void APTWGameState::OnRep_Teams()
{
	
}

void APTWGameState::OnRep_WinTeamId()
{
	
}

void APTWGameState::DecreaseTimer()
{
	if (!HasAuthority()) return;

	UE_LOG(Log_GameState, Log, TEXT("Timer: %d"), RemainTime);
	
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
	GameData.CurrentRound = CurrentRound;
	UE_LOG(LogTemp, Warning, TEXT("Current Round: %d"), CurrentRound);
}

void APTWGameState::AdvanceMiniGameRound()
{
	CurrentMiniGameRound++;
}

void APTWGameState::AddChaosItemEntry(const FPTWChaosItemEntry& Entry)
{
	GameData.ChaosItemEntries.Add(Entry);
}

void APTWGameState::ResetChaosItemEntries()
{
	GameData.ChaosItemEntries.Empty();
}

void APTWGameState::AddPlayedMap(FName MapRowName)
{
	GameData.PlayedMapRowNames.Add(MapRowName);
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
	GameData.CurrentRound = NewRound;
	CurrentRound = NewRound;
	
	OnRep_CurrentRound();
}

void APTWGameState::SetCurrentPhase(EPTWGamePhase NewGamePhase)
{
	if (!HasAuthority()) return;

	CurrentGamePhase = NewGamePhase;

	OnRep_CurrentGamePhase();

	UE_LOG(LogTemp, Warning, TEXT(
		"[Broadcast][PTWMiniGameTitle] GS: %p"),
		this
	);
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

void APTWGameState::SetWinTeamId(int32 TeamId)
{
	if (!HasAuthority()) return;

	WinTeamId = TeamId;
}

void APTWGameState::SetCurrentMiniGameRule(const FPTWMiniGameRule& Rule)
{
	CurrentMiniGameRule = Rule;
}

void APTWGameState::AddLobbyRankingDataMap(const TMap<FString, FPTWPlayerGameData>& InData)
{
	if (!HasAuthority()) return;

	LobbyRankingDataMap.Empty();
	
	for (const auto& Pair : InData)
	{
		const FString& PlayerId = Pair.Key;
		const FPTWPlayerGameData& PlayerGameData = Pair.Value;
		
		FPTWLobbyRankingData LobbyData;
		LobbyData.PlayerName = PlayerGameData.PlayerData.PlayerName;
		LobbyData.Score = PlayerGameData.PlayerData.TotalWinPoints;
		LobbyData.Gold = PlayerGameData.PlayerData.Gold;
		LobbyData.InventoryItemIDs = PlayerGameData.PlayerData.InventoryItemIDs;
		
		LobbyRankingDataMap.Add(PlayerId, LobbyData);
	}

	SortLobbyRankingData();

}
void APTWGameState::UpdateLobbyRankingDataMap(const FString& PlayerId, const FPTWPlayerData& PlayerData)
{
	if (!HasAuthority()) return;
	
	if (!LobbyRankingDataMap.Contains(PlayerId)) return;

	FPTWLobbyRankingData* LobbyPlayerData = LobbyRankingDataMap.Find(PlayerId);
	if (!LobbyPlayerData) return;

	LobbyPlayerData->Score = PlayerData.TotalWinPoints;
	LobbyPlayerData->Gold = PlayerData.Gold;
	//LobbyPlayerData->InventoryItemIDs = PlayerData.InventoryItemIDs;

	SortLobbyRankingData();
}

void APTWGameState::SortLobbyRankingData()
{
	LobbyRankingData.Empty();

	LobbyRankingDataMap.GenerateValueArray(LobbyRankingData);
	
	LobbyRankingData.Sort([](const FPTWLobbyRankingData& A, const FPTWLobbyRankingData& B)
	{
		if (A.Score != B.Score)
		{
			return A.Score > B.Score;
		}

		return A.Gold > B.Gold;
	});
	
	for (int32 i = 0; i < LobbyRankingData.Num(); ++i)
	{
		if (i == 0)
		{
			LobbyRankingData[i].Rank = 1;
			continue;
		}

		const auto& Prev = LobbyRankingData[i - 1];
		auto& Curr = LobbyRankingData[i];

		if (Curr.Score == Prev.Score && Curr.Gold == Prev.Gold)
		{
			Curr.Rank = Prev.Rank;
		}
		else
		{
			Curr.Rank = i + 1;
		}
	}
}

void APTWGameState::AddMiniGameRankingDataMap(const TMap<FString, FString>& InData)
{
	if (!HasAuthority()) return;

	MiniGameRankingDataMap.Empty();

	for (const auto& Pair : InData)
	{
		const FString& PlayerId = Pair.Key;
		const FString& PlayerName = Pair.Value;

		FPTWMiniGameRankingData MiniGameData;
		MiniGameData.PlayerName = PlayerName;
		
		MiniGameRankingDataMap.Add(PlayerId, MiniGameData);
	}

	SortMiniGameRankingData();
}

void APTWGameState::UpdateMiniGameRankingDataMap(APlayerState* InPlayerState)
{
	if (!HasAuthority()) return;

	APTWPlayerState* PlayerState = Cast<APTWPlayerState>(InPlayerState);
	if (!PlayerState) return;
	
	if (!MiniGameRankingDataMap.Contains(PlayerState->GetUniqueId().ToString())) return;

	FPTWMiniGameRankingData* RankingData = MiniGameRankingDataMap.Find(PlayerState->GetUniqueId().ToString());
	if (!RankingData) return;

	RankingData->Score = PlayerState->GetPlayerRoundData().Score;
	RankingData->Kill = PlayerState->GetPlayerRoundData().KillCount;
	RankingData->Death = PlayerState->GetPlayerRoundData().DeathCount;
	RankingData->DeathOrder = PlayerState->GetPlayerRoundData().DeathOrder;
	//MiniGameRankingData->InventoryItemIDs = PlayerData.InventoryItemIDs;

	SortMiniGameRankingData();
}

void APTWGameState::SortMiniGameRankingData()
{
	MiniGameRankingData.Empty();
	MiniGameRankingDataMap.GenerateValueArray(MiniGameRankingData);
	
	if (CurrentMiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
	{
		MiniGameRankingData.Sort([](const FPTWMiniGameRankingData& A, const FPTWMiniGameRankingData& B)
		{
			const bool bAAlive = (A.DeathOrder == 0);
			const bool bBAlive = (B.DeathOrder == 0);

			
			// 1. 생존자 우선
			if (bAAlive != bBAlive)
			{
				return bAAlive;
			}

			// 2. 둘 다 생존 → Score
			if (bAAlive)
			{
				if (A.Score != B.Score)
				{
					return A.Score > B.Score;
				}

				return A.Kill > B.Kill;
			}

			// 3. 둘 다 사망 → 늦게 죽은 순
			if (A.DeathOrder != B.DeathOrder)
			{
				return A.DeathOrder > B.DeathOrder;
			}
			
			if (A.Score != B.Score)
			{
				return A.Score > B.Score;
			}

			return A.Kill > B.Kill;
		});
	}
	else
	{
		MiniGameRankingData.Sort([](const FPTWMiniGameRankingData& A, const FPTWMiniGameRankingData& B)
		{
			if (A.bLeftGame != B.bLeftGame)
			{
				return !A.bLeftGame;
			}

			// 1. Score
			if (A.Score != B.Score)
			{
				return A.Score > B.Score;
			}

			// 2. Kill
			if (A.Kill != B.Kill)
			{
				return A.Kill > B.Kill;
			}

			// 3. Death (적을수록 좋음)
			return A.Death < B.Death;
		});
	}

	// Rank 계산
	for (int32 i = 0; i < MiniGameRankingData.Num(); ++i)
	{
		if (i == 0)
		{
			MiniGameRankingData[i].Rank = 1;
			continue;
		}

		const auto& Prev = MiniGameRankingData[i - 1];
		auto& Curr = MiniGameRankingData[i];

		bool bSameRank = false;

		if (CurrentMiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
		{
			bSameRank =
				(Curr.DeathOrder == Prev.DeathOrder) &&
				(Curr.Score == Prev.Score) &&
				(Curr.Kill == Prev.Kill);
		}
		else
		{
			bSameRank =
				(Curr.Score == Prev.Score) &&
				(Curr.Kill == Prev.Kill) &&
				(Curr.Death == Prev.Death);
		}

		if (bSameRank)
		{
			Curr.Rank = Prev.Rank;
		}
		else
		{
			Curr.Rank = i + 1;
		}
	}
	
}

void APTWGameState::Server_SetPropSeed_Implementation(int32 NewSeed)
{
	if (!HasAuthority()) return;

	PropSeed = NewSeed;
	
	OnRep_PropSeed();
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

void APTWGameState::Multicast_BroadcastMiniGameEnded_Implementation()
{
	if (OnMiniGameEnded.IsBound())
	{
		OnMiniGameEnded.Broadcast();
	}
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

void APTWGameState::Multicast_SystemMessage_Implementation(const FString& Message)
{
	OnChatMessageBroadcast.Broadcast(TEXT("[System]"), Message);
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

	UE_LOG(LogTemp, Warning, TEXT(
		"[Broadcast][PTWMiniGameTitle] GS: %p"),
		this
	);
}

void APTWGameState::OnRep_RankedPlayers()
{
	OnUpdateRankedPlayers.Broadcast(RankedPlayers);
}

void APTWGameState::OnRep_LobbyRankingData()
{
	OnUpdateLobbyRankingData.Broadcast(LobbyRankingData);
}

void APTWGameState::OnRep_MiniGameRankingData()
{
	OnUpdateMiniGameRankingData.Broadcast(MiniGameRankingData);
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

void APTWGameState::OnRep_PropSeed()
{
	if (UWorld* World = GetWorld())
	{
		if (auto* PropSubsys = World->GetSubsystem<UPTWPropSubsystem>())
		{
			PropSubsys->ApplyPropDataSeeded(PropData, PropSeed);
		}
	}
}

void APTWGameState::Server_SetPropData_Implementation(UPTWPropData* NewPropData)
{
	if (!HasAuthority()) return;

	PropData = NewPropData;
	
	OnRep_PropData();
}

void APTWGameState::OnRep_PropData()
{
	if (UWorld* World = GetWorld())
	{
		if (auto* PropSubsys = World->GetSubsystem<UPTWPropSubsystem>())
		{
			PropSubsys->ApplyPropDataSeeded(PropData, PropSeed);
		}
	}
}
