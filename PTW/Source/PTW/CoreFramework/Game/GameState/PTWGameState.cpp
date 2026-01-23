// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameState.h"

#include "Net/UnrealNetwork.h"

APTWGameState::APTWGameState()
{
	bReplicates = true;
}


void APTWGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWGameState, RemainTime);
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
	UE_LOG(LogTemp, Warning, TEXT("Timer: %d"), RemainTime);
}

void APTWGameState::AdvanceRound()
{
	CurrentRound++;	
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

	UE_LOG(LogTemp, Warning, TEXT("Current Round: %d"), CurrentRound);
}

void APTWGameState::OnRep_RemainTime()
{
	OnRemainTimeChanged.Broadcast(RemainTime);
}

void APTWGameState::OnRep_CurrentRound()
{
	
}
