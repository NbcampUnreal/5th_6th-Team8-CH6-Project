// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWGhostChaseMiniGameMode.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "System/PTWItemSpawnManager.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"
#include "CoreFramework/PTWPlayerController.h"

APTWGhostChaseMiniGameMode::APTWGhostChaseMiniGameMode()
{
	// 규칙 설정
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Survival;

	MiniGameRule.TimeRule.bUseCountDown = true;
	MiniGameRule.TimeRule.CountDown = 10.f; // 10초 대기

	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Timer = 180.f;    // 3분 게임
}

void APTWGhostChaseMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);

	AController* DeadController = nullptr;

	if (APawn* DeadPawn = Cast<APawn>(DeadActor))
	{
		DeadController = DeadPawn->GetController();
	}

	if (DeadController)
	{
		OnPlayerEliminated(DeadController);
	}
}

void APTWGhostChaseMiniGameMode::OnPlayerEliminated(AController* EliminatedController)
{
	if (!EliminatedController) return;

	if (auto* GCComp = EliminatedController->FindComponentByClass<UPTWGhostChaseControllerComponent>())
	{
		GCComp->SetTarget(nullptr);
	}

	if (ActiveChasers.Num() == 0) return;

	int32 ElimIndex = ActiveChasers.Find(EliminatedController);
	if (ElimIndex == INDEX_NONE) return;

	if (ActiveChasers.Num() <= 1)
	{
		ActiveChasers.RemoveAt(ElimIndex);
		TargetMap.Remove(EliminatedController);
		EndGame();
		return;
	}

	int32 NumChasers = ActiveChasers.Num();

	// 누구의 타겟이었는지 찾기 (내 앞 사람)
	int32 ChaserIndex = (ElimIndex - 1 + NumChasers) % NumChasers;
	AController* ChaserOfDeadPlayer = ActiveChasers[ChaserIndex];

	// 나의 다음 타겟 찾기 (내 뒷 사람)
	int32 NewTargetIndex = (ElimIndex + 1) % NumChasers;
	AController* NewTargetForChaser = ActiveChasers[NewTargetIndex];

	// 배열에서 제거
	ActiveChasers.RemoveAt(ElimIndex);
	TargetMap.Remove(EliminatedController);

	// 타겟 재설정: 죽은 자를 쫓던 사람(Chaser)이 죽은 자의 타겟(NewTarget)을 쫓게 함
	if (ChaserOfDeadPlayer && NewTargetForChaser && ChaserOfDeadPlayer != NewTargetForChaser)
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

bool APTWGhostChaseMiniGameMode::IsValidChaseTarget(AController* Chaser, AController* Target) const
{
	if (!Chaser || !Target)
	{
		return false;
	}

	const AController* const* FoundTarget = TargetMap.Find(Chaser);

	if (!FoundTarget)
	{
		return false;
	}

	return (*FoundTarget == Target);
}

void APTWGhostChaseMiniGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void APTWGhostChaseMiniGameMode::EndGame()
{
	if (bIsGameEnded) return;

	if (!PTWGameState) return;

	// 순위 업데이트 수행
	// WinType이 Survival이므로 [생존자 -> 마지막 사망자 -> ... -> 최초 사망자] 순으로 정렬됨
	PTWGameState->UpdateRanking(MiniGameRule);

	// 점수 계산 직전에만 잠시 WinType을 Target으로 변경
	EPTWWinType OriginalType = MiniGameRule.WinConditionRule.WinType;
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Target;

	// 내부에서 PTWGameState->ApplyMiniGameRankScore(MiniGameRule)가 실행
	Super::EndGame();

	// 원래 타입으로 복구
	MiniGameRule.WinConditionRule.WinType = OriginalType;
}

void APTWGhostChaseMiniGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();

	// 필요 시 대기 시간 동안 플레이어 움직임을 제한하는 등의 로직 추가
}

void APTWGhostChaseMiniGameMode::StartRound()
{
	Super::StartRound();

	// 타겟 체인 구성
	SetupTargetChain();

	// 기본무기 지급
	GiveBaseWeaponToAll();

	// 닉네임 구분시작
	StartNameDistinguish();

	// 투명화 적용
	//ApplyInvisibilityToAll();
}

void APTWGhostChaseMiniGameMode::SetupTargetChain()
{
	ActiveChasers.Empty();
	TargetMap.Empty();

	// 월드의 모든 플레이어 컨트롤러 수집
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC) && IsValid(PC->GetPawn()))
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
	
}

void APTWGhostChaseMiniGameMode::UpdatePlayerTargetUI(AController* Chaser, AController* NewTarget)
{
	if (!IsValid(Chaser) || !IsValid(NewTarget)) return;

	// Target의 Pawn
	APawn* TargetPawn = NewTarget->GetPawn();

	if (IsValid(Chaser))
	{
		if (auto* GCComp = Chaser->FindComponentByClass<UPTWGhostChaseControllerComponent>())
		{
			GCComp->SetTarget(TargetPawn);
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

void APTWGhostChaseMiniGameMode::StartNameDistinguish()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			// PC 내부의 UI 컴포넌트를 찾아 초기화 함수 호출
			if (auto* UIComp = PC->FindComponentByClass<UPTWUIControllerComponent>())
			{
				UIComp->Client_FindGhostChaseComponent();
			}
		}
	}
}
