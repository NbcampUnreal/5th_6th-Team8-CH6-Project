// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWRedLightGameMode.h"
#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void APTWRedLightGameMode::AssignTagger(APlayerController* TaggerPC, TSubclassOf<APTWRedLightCharacter> TaggerClass)
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
