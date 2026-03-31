// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWRedLightGameMode.h"
#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "System/PTWItemSpawnmanager.h"
#include "EngineUtils.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameplayEffect.h"

void APTWRedLightGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (CachedBlueprintClass)
	{
		CachedBlueprintInstance = UGameplayStatics::GetActorOfClass(GetWorld(), CachedBlueprintClass);

		if (CachedBlueprintInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] 레벨에서 블루프린트 액터 캐싱 성공: %s"), *CachedBlueprintInstance->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[GameMode] 레벨에 해당 클래스(%s)의 액터가 배치되어 있지 않습니다! 맵에 액터를 드래그해서 올려주세요."), *CachedBlueprintClass->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] 검색할 블루프린트 클래스가 설정되지 않았습니다! BP_PTWRedLightGameMode의 디테일 패널을 확인하세요."));
	}
}

void APTWRedLightGameMode::AssignTagger(APlayerController* TaggerPC)
{
	if (!TaggerPC || !TaggerClass) return;

	if (APawn* OldPawn = TaggerPC->GetPawn())
	{
		TaggerPC->UnPossess();
		OldPawn->Destroy();
	}

	AActor* DollStart = FindPlayerStart(TaggerPC, TEXT("DollSpawn"));
	FVector SpawnLocation = DollStart ? DollStart->GetActorLocation() : FVector(0.f, 0.f, 200.f);
	FRotator SpawnRotator = DollStart->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (APTWRedLightCharacter* DollPawn = GetWorld()->SpawnActor<APTWRedLightCharacter>(TaggerClass, SpawnLocation, SpawnRotator, SpawnParams))
	{
		TaggerPC->Possess(DollPawn);
		CurrentTagger = DollPawn;
	}
}

void APTWRedLightGameMode::OnRedLightStateChanged(bool bIsRedLight, APTWRedLightCharacter* TaggerChar)
{
	CurrentTagger = TaggerChar;

	ReceiveOnPhaseChanged(bIsRedLight);

	if (bIsRedLight)
	{
		GetWorldTimerManager().SetTimer(MovementCheckTimer, this, &APTWRedLightGameMode::CheckPlayerMovements, 0.1f, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(MovementCheckTimer);

		for (ACharacter* CaughtPlayer : CaughtPlayers)
		{
			if (IsValid(CaughtPlayer))
			{
				if (InvincibleEffectClass)
				{
					if (IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(CaughtPlayer))
					{
						FGameplayEffectContextHandle EmptyContext;
						CombatInterface->ApplyGameplayEffectToSelf(InvincibleEffectClass, 1.0f, EmptyContext);
					}
				}
				if (CurrentTagger)
				{
					CurrentTagger->Multicast_RemoveSpottedMark(CaughtPlayer);
				}
			}
		}

		CaughtPlayers.Empty();
	}
}

void APTWRedLightGameMode::CheckPlayerMovements()
{
	if (!CurrentTagger) return;

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!Controller) continue;

		ACharacter* Runner = Cast<ACharacter>(Controller->GetPawn());

		if (Runner && Runner != CurrentTagger && !CaughtPlayers.Contains(Runner))
		{
			float CurrentSpeed = Runner->GetVelocity().Size();

			if (CurrentSpeed > MaxAllowedSpeed)
			{
				CaughtPlayers.Add(Runner);
				UE_LOG(LogTemp, Warning, TEXT("🚨 [RedLight] %s 발각됨! (무적 해제 및 저격 대기)"), *Runner->GetName());

				CurrentTagger->Multicast_SpottedPlayer(Runner);

				if (IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(Runner))
				{
					CombatInterface->RemoveEffectWithTag(InvincibleTag);
				}
			}
		}
	}
}

bool APTWRedLightGameMode::IsPlayerCaught(ACharacter* PlayerToCheck) const
{
	return CaughtPlayers.Contains(PlayerToCheck);
}

void APTWRedLightGameMode::StartRound()
{
	Super::StartRound();

	for (AActor* ActorToDestroy : ActorsToDestroyOnStart)
	{
		if (IsValid(ActorToDestroy))
		{
			ActorToDestroy->Destroy();
		}
	}
	ActorsToDestroyOnStart.Empty();

	TArray<APlayerController*> ValidPlayers;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			ValidPlayers.Add(PC);
		}
	}

	if (ValidPlayers.Num() > 0 && TaggerClass)
	{
		int32 RandomIndex = FMath::RandRange(0, ValidPlayers.Num() - 1);
		APlayerController* SelectedPC = ValidPlayers[RandomIndex];

		AssignTagger(SelectedPC);

		FString TaggerName = SelectedPC->PlayerState ? SelectedPC->PlayerState->GetPlayerName() : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("[RedLight] 라운드 시작! %s 플레이어가 술래로 당첨되었습니다!"), *TaggerName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[RedLight] 에러: 참가 중인 플레이어가 없거나 TaggerClass가 설정되지 않았습니다!"));
	}

	if (InvincibleEffectClass)
	{
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AController* Controller = It->Get();
			if (!Controller) continue;

			ACharacter* Runner = Cast<ACharacter>(Controller->GetPawn());

			if (Runner && Runner != CurrentTagger)
			{
				if (IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(Runner))
				{
					FGameplayEffectContextHandle EmptyContext;
					CombatInterface->ApplyGameplayEffectToSelf(InvincibleEffectClass, 1.0f, EmptyContext);
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[RedLight] 모든 도망자에게 기본 무적 상태가 부여되었습니다."));
	}
}

AActor* APTWRedLightGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> NormalStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* Start = *It;
		if (Start)
		{
			if (Start->PlayerStartTag != TEXT("DollSpawn"))
			{
				NormalStarts.Add(Start);
			}
		}
	}

	if (NormalStarts.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, NormalStarts.Num() - 1);
		return NormalStarts[RandomIndex];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void APTWRedLightGameMode::PlayerFinished(ACharacter* FinishedPlayer)
{
	if (!FinishedPlayer) return;

	APTWPlayerController* PC = FinishedPlayer->GetController<APTWPlayerController>();
	APTWPlayerState* PS = FinishedPlayer->GetPlayerState<APTWPlayerState>();

	if (PC && PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RedLight] %s 플레이어가 결승선에 도착했습니다! (생존 성공)"), *PS->GetPlayerName());

		AddRoundScore(PS, 10);

		if (PTWGameState)
		{
			PTWGameState->AlivePlayers.Remove(PS);
		}

		FinishedPlayer->SetActorHiddenInGame(true);
		FinishedPlayer->SetActorEnableCollision(false);

		PC->StartSpectating();

		CheckEndGameCondition();
	}
}
