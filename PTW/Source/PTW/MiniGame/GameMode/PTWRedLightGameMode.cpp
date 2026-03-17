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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (APTWRedLightCharacter* DollPawn = GetWorld()->SpawnActor<APTWRedLightCharacter>(TaggerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		TaggerPC->Possess(DollPawn);
		CurrentTagger = DollPawn;
	}
}

void APTWRedLightGameMode::OnRedLightStateChanged(bool bIsRedLight, APTWRedLightCharacter* TaggerChar)
{
	CurrentTagger = TaggerChar;

	if (bIsRedLight)
	{
		GetWorldTimerManager().SetTimer(MovementCheckTimer, this, &APTWRedLightGameMode::CheckPlayerMovements, 0.1f, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(MovementCheckTimer);
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
				UE_LOG(LogTemp, Warning, TEXT("[RedLight] %s 발각됨! (저격 대기)"), *Runner->GetName());

				CurrentTagger->Multicast_SpottedPlayer(Runner);
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

		if (TaggerWeaponDef)
		{
			if (APTWPlayerState* PS = SelectedPC->GetPlayerState<APTWPlayerState>())
			{
				if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
				{
					SpawnManager->SpawnSingleItem(PS, TaggerWeaponDef);
					UE_LOG(LogTemp, Warning, TEXT("[RedLight] 술래에게 전용 무기가 지급되었습니다."));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[RedLight] TaggerWeaponDef(지급할 무기)가 설정되지 않아 무기를 지급하지 않았습니다."));
		}

		FString TaggerName = SelectedPC->PlayerState ? SelectedPC->PlayerState->GetPlayerName() : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("[RedLight] 라운드 시작! %s 플레이어가 술래로 당첨되었습니다!"), *TaggerName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[RedLight] 에러: 참가 중인 플레이어가 없거나 TaggerClass가 설정되지 않았습니다!"));
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
