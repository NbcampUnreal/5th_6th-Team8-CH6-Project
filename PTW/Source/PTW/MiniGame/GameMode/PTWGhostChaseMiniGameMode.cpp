// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWGhostChaseMiniGameMode.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "System/PTWItemSpawnManager.h"
#include "Inventory/PTWInventoryComponent.h"

APTWGhostChaseMiniGameMode::APTWGhostChaseMiniGameMode()
{
	// 규칙 설정
	MiniGameRule.TimeRule.bUseCountDown = true;
	MiniGameRule.TimeRule.CountDown = 10.f; // 10초 대기

	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Timer = 180.f;    // 3분 게임
}

void APTWGhostChaseMiniGameMode::OnPlayerEliminated(AController* EliminatedController)
{
	if (!EliminatedController || ActiveChasers.Num() <= 1) return;

	int32 ElimIndex = ActiveChasers.Find(EliminatedController);
	if (ElimIndex == INDEX_NONE) return;

	// 누구의 타겟이었는지 찾기 (내 앞 사람)
	int32 ChaserIndex = (ElimIndex - 1 + ActiveChasers.Num()) % ActiveChasers.Num();
	AController* ChaserOfDeadPlayer = ActiveChasers[ChaserIndex];

	// 나의 다음 타겟 찾기 (내 뒷 사람)
	int32 NewTargetIndex = (ElimIndex + 1) % ActiveChasers.Num();
	AController* NewTargetForChaser = ActiveChasers[NewTargetIndex];

	// 배열에서 제거
	ActiveChasers.RemoveAt(ElimIndex);

	// 타겟 재설정: 죽은 자를 쫓던 사람(Chaser)이 죽은 자의 타겟(NewTarget)을 쫓게 함
	if (ChaserOfDeadPlayer != NewTargetForChaser) // 1대1 상황이 아니면
	{
		TargetMap.Add(ChaserOfDeadPlayer, NewTargetForChaser);
		UpdatePlayerTargetUI(ChaserOfDeadPlayer, NewTargetForChaser);
	}

	TargetMap.Remove(EliminatedController);

	// 최후의 1인 체크
	if (ActiveChasers.Num() == 1)
	{
		EndGame();
	}
}

void APTWGhostChaseMiniGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void APTWGhostChaseMiniGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();

	// 타겟 체인 구성
	SetupTargetChain();

	// 필요 시 대기 시간 동안 플레이어 움직임을 제한하는 등의 로직 추가
}

void APTWGhostChaseMiniGameMode::StartRound()
{
	Super::StartRound();

	// 기본무기 지급
	GiveBaseWeaponToAll();

	// 투명화 적용
	ApplyInvisibilityToAll();
}

void APTWGhostChaseMiniGameMode::SetupTargetChain()
{
	ActiveChasers.Empty();
	TargetMap.Empty();

	// 월드의 모든 플레이어 컨트롤러 수집
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (AController* PC = It->Get())
		{
			ActiveChasers.Add(PC);
		}
	}

	if (ActiveChasers.Num() < 2) return;

	// 랜덤하게 섞기
	const int32 NumActivePlayers = ActiveChasers.Num();
	for (int32 i = 0; i < NumActivePlayers; i++)
	{
		int32 DestIndex = FMath::RandRange(i, NumActivePlayers - 1);
		ActiveChasers.Swap(i, DestIndex);
	}

	// 순환 구조 연결 (i -> i+1)
	for (int32 i = 0; i < NumActivePlayers; i++)
	{
		AController* Chaser = ActiveChasers[i];
		AController* Target = ActiveChasers[(i + 1) % NumActivePlayers];

		TargetMap.Add(Chaser, Target);
		UpdatePlayerTargetUI(Chaser, Target);
	}
}

void APTWGhostChaseMiniGameMode::ApplyInvisibilityToAll()
{
	for (AController* PC : ActiveChasers)
	{
		if (APTWPlayerCharacter* TargetChar = Cast<APTWPlayerCharacter>(PC->GetPawn()))
		{
			TargetChar->ApplyInvisibilityEffect(InvisibilityEffectClass);
			//TargetChar->ApplyInvisibilityEffect(test);
		}
	}
}

void APTWGhostChaseMiniGameMode::UpdatePlayerTargetUI(AController* Chaser, AController* NewTarget)
{
	if (!Chaser || !NewTarget) return;

	// Chaser의 PlayerState
	APTWPlayerState* ChaserPS = Chaser->GetPlayerState<APTWPlayerState>();
	// Target의 Pawn
	APawn* TargetPawn = NewTarget->GetPawn();

	if (ChaserPS && TargetPawn)
	{
		// 이 변수가 Replicated이므로, 서버에서 설정하면 클라이언트의 OnRep_CurrentTargetPawn이 실행됩니다.
		ChaserPS->CurrentTargetPawn = TargetPawn;

		// 만약 서버가 호스트 플레이어라면 OnRep이 자동으로 안 돌 수 있으니 직접 호출
		if (Chaser->IsLocalController())
		{
			ChaserPS->OnRep_CurrentTargetPawn();
		}
	}
}

void APTWGhostChaseMiniGameMode::GiveBaseWeaponToAll()
{
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;

	// 현재 게임에 참여 중인 모든 추격자(플레이어) 순회
	for (AController* Controller : ActiveChasers)
	{
		if (!Controller) continue;

		APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(Controller->GetPawn());
		APTWPlayerState* PS = Controller->GetPlayerState<APTWPlayerState>();

		if (PC && PS)
		{
			// 아이템 스폰
			SpawnManager->SpawnSingleItem(PS, GhostWeaponDef);

			// 무기 지급
			if (UPTWInventoryComponent* Inven = PC->GetInventoryComponent())
			{
				Inven->EquipWeapon(0);
			}
		}
	}
}
