// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWBombMiniGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/PTWPlayerState.h"
#include "PTW/MiniGame/Item/PTWBombActor.h"

void APTWBombMiniGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentRound = 0;
	StartRound();
}

void APTWBombMiniGameMode::StartRound()
{
	CurrentRound++;

	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d / %d - Countdown Start"), CurrentRound, MaxRoundCount);
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(true);
	}
	
	StartCountDown();
}

void APTWBombMiniGameMode::OnCountDownFinished()
{
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(false);
	}
	
	AssignRandomBombOwner();
	
	if (!BombActor && BombActorClass)
	{
		BombActor = GetWorld()->SpawnActor<APTWBombActor>(
			BombActorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator
		);
	}
	
	if (BombActor && BombOwnerPS)
	{
		APawn* OwnerPawn = BombOwnerPS->GetPawn();
		BombActor->SetBombOwner(OwnerPawn);
	}

	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Play Start"), CurrentRound);

	// 라운드 진행 타이머 시작 
	GetWorldTimerManager().SetTimer(RoundTimerHandle,this,&APTWBombMiniGameMode::EndTimer,RoundPlayTime,false);
}

void APTWBombMiniGameMode::EndTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Explosion Timing"), CurrentRound);
	
	CleanupBombActor();

	// 3회 다 돌면 미니게임 종료
	if (CurrentRound >= MaxRoundCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombMode] Finished"));
		
		Super::EndTimer();
		return;
	}

	// 다음 라운드 진행
	StartRound();
}

void APTWBombMiniGameMode::CleanupBombActor()
{
	if (BombActor)
	{
		BombActor->Destroy();
		BombActor = nullptr;
	}
}

void APTWBombMiniGameMode::GetAlivePlayerStates(TArray<APTWPlayerState*>& OutAlive) const
{
	OutAlive.Reset();

	const AGameStateBase* GSBase = GetGameState<AGameStateBase>();
	if (!GSBase) return;
	
	for (APlayerState* PS : GSBase->PlayerArray)
	{
		APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS);
		if (!PTWPS) continue;

		if (PTWPS->IsOnlyASpectator()) continue;
		if (PTWPS->IsInactive()) continue;

		OutAlive.Add(PTWPS);
	}
}

void APTWBombMiniGameMode::AssignRandomBombOwner()
{
	TArray<APTWPlayerState*> AlivePlayers;
	GetAlivePlayerStates(AlivePlayers);

	if (AlivePlayers.Num() <= 0)
	{
		BombOwnerPS = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - BombOwner assign failed (no alive players)"), CurrentRound);
		return;
	}

	const int32 PickIndex = FMath::RandRange(0, AlivePlayers.Num() - 1);
	BombOwnerPS = AlivePlayers[PickIndex];

	const FString OwnerName = BombOwnerPS ? BombOwnerPS->GetPlayerName() : TEXT("None");
	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - BombOwner = %s"), CurrentRound, *OwnerName);
}
