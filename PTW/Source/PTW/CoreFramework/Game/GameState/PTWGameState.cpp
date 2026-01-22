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
	UE_LOG(LogTemp, Warning, TEXT("Timer: %d"), RemainTime);
	if (RemainTime <= 0)
	{
		OnTimerFinished.Broadcast();
	}
	else
	{
		RemainTime--;
	}
}

void APTWGameState::OnRep_RemainTime()
{
	OnRemainTimeChanged.Broadcast(RemainTime);
}
