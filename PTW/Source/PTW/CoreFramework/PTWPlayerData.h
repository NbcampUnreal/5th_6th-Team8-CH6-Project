// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWPlayerData.generated.h"

/**
 * 플레이어의 누적 진행 데이터를 저장하는 구조체
 * - 게임 전반에서 공통으로 사용되는 플레이어 상태 정보
 * - 점수, 재화 등 라운드 간 유지되는 데이터 포함
 */
USTRUCT(BlueprintType)
struct FPTWPlayerData
{
	GENERATED_BODY()

	/** 플레이어 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "";

	/** 플레이어의 누적 승점 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalWinPoints = 0;

	/** 플레이어가 보유한 재화(골드) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gold = 0;

	// 향후 인벤토리 시스템 확장 시 사용 예정
	// TArray<AActor> InventoryItem;
};

/**
 * 한 라운드 동안 플레이어의 전투 통계를 저장하는 구조체
 * - 라운드 종료 시 집계 및 승리 조건 판정에 사용
 */
USTRUCT(BlueprintType)
struct FPTWPlayerRoundData
{
	GENERATED_BODY()

	/** 라운드 중 처치한 적 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 KillCount = 0;
	
	/** 라운드 중 사망한 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeathCount = 0;
	
	/** 라운드에서 획득한 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = 0;

};
