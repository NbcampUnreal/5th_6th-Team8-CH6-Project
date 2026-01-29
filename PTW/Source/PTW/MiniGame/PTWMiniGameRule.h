// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWMiniGameRule.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPTWKillRule
{
	GENERATED_BODY()

	/** 킬 1회당 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kill")
	int32 KillScore = 1;
	
	/** 데스 1회당 감점 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kill")
	int32 DeathPenalty = 0;
	
};

/**
 * 플레이어 스폰 및 리스폰 동작에 대한 규칙 정의 구조체
 * - 리스폰 사용 여부
 * - 팀 스폰 적용 여부
 * - 리스폰 지연 및 스폰 보호 시간 설정
 */
USTRUCT(BlueprintType)
struct FPTWSpawnRule
{
	GENERATED_BODY()

	/** 리스폰 시스템 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bUseRespawn = true;	
	
	/** 팀 기반 스폰 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bUseRespawn"))
	bool bUseTeamSpawn = false;

	/** 리스폰까지 대기 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bUseRespawn"))
	float RespawnDelay = 5.f;

	/** 스폰 보호(무적) 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bUseSpawnProtection = true;

	/** 스폰 보호 지속 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "bUseRespawn"))
	float SpawnProtectionTime = 3.f;
};

/**
 * 플레이어 시작 장비(Loadout) 및 무기 제한 규칙 정의 구조체
 * - 스폰 시 기본 무기 지급 여부
 * - 사망 시 무기 드랍 처리
 * - 사용 가능한 무기 제한 규칙
 */
USTRUCT(BlueprintType)
struct FPTWLoadoutRule
{
	GENERATED_BODY()
	
	/** 스폰 시 기본 무기 지급 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	bool bGiveDefaultWeaponOnSpawn = true;

	/** 기본 지급 무기 목록 (여러 개 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout", meta = (EditCondition = "bGiveDefaultWeaponOnSpawn"))
	TArray<TObjectPtr<class UPTWItemDefinition>> DefaultWeapon;
	
	/** 사망 시 무기 드랍 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	bool bDropWeaponOnDeath = true;
	
	/** 허용된 무기 태그 (비어 있으면 전부 허용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FGameplayTagContainer AllowedWeapon;
	
};

/**
 * 전투 관련 행동 및 무기 사용 제한 규칙 정의 구조체
 * - 무기/공격 수단 사용 허용 여부
 * - 탄약 및 재장전/무기 교체 제한
 */
USTRUCT(BlueprintType)
struct FPTWCombatRule
{
	GENERATED_BODY()
	
	/** 총기 사용 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAllowGun = true;
	
	/** 투척 무기(수류탄 등) 사용 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAllowGrenade = true;
	
	/** 근접 공격 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAllowMelee = true;

	/** 탄약 무한 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat",  meta = (EditCondition = "bAllowGun"))
	bool bInfinityAmmo = false;

	/** 재장전 금지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bDisableReload = false;

	/** 무기 교체(스왑) 금지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bDisableWeaponSwap = false;
	
};

/**
 * 플레이어 이동 관련 규칙 정의 구조체
 * - 이동 행동 허용 여부
 * - 이동 속도 및 중력 배율 조정
 * - 플레이어 간 충돌 설정
 */
USTRUCT(BlueprintType)
struct FPTWMovementRule
{
	GENERATED_BODY()
	
	/** 점프 허용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bAllowJump = true;

	/** 달리기 허용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Movement")
	bool bAllowSprint = true;

	/** 이동 속도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Movement", meta=(ClampMin="0.1", ClampMax="3.0"))
	float MoveSpeedScale = 1.0f;

	/** 중력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Movement", meta=(ClampMin="0.1", ClampMax="3.0"))
	float GravityScale = 1.0f;

	/** 플레이어끼리 충돌 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnablePlayerCollision = true;
};

/**
 * 게임 승리 조건 유형
 * - 미니게임 또는 라운드 종료 시 승패 판정 기준
 */
UENUM(BlueprintType)
enum class EPTWWinType : uint8
{
	/** 기본 값 */
	None UMETA(DisplayName = "None"),
	
	/** 제한 시간 종료 시 판정 */
	TimeLimit UMETA(DisplayName="Time Limit"),

	 /** 목표 달성 시 즉시 종료 */
	Target UMETA(DisplayName="Target"),

	/** 마지막까지 생존한 플레이어가 승리 */
	Survival UMETA(DisplayName = "Survival"),

	/** 마지막까지 생존한 팀이 승리 */
	TeamSurvival UMETA(DisplayName = "Team Survival"),

	/** 미션 목표를 먼저 달성하면 승리 */
	Mission UMETA(DisplayName = "Mission")
};


UENUM(BlueprintType)
enum class EPTWWinMetric : uint8
{
	None UMETA(DisplayName="None"),
	KillCount UMETA(DisplayName="Kill Count"),
    Score UMETA(DisplayName="Score")
};


/**
 * 연장전(Overtime) 진행 규칙
 * - 정규 시간 종료 후 승패가 결정되지 않았을 때 적용
 * - 승리 조건을 보조하거나 시간을 연장하는 방식 정의
 */
UENUM(BlueprintType)
enum class EPTWOvertimeRule : uint8
{
	/** 연장전 없음 */
	None            UMETA(DisplayName="No Overtime"),

	/** 한 번이라도 처치가 발생하면 종료 */
	SuddenDeath     UMETA(DisplayName="Sudden Death"),
	
	/** 첫 킬을 달성한 플레이어(또는 팀) 승리 */
	FirstKill       UMETA(DisplayName="First Kill Wins"),
	
	/** 첫 점수를 달성한 플레이어(또는 팀) 승리 */
	FirstScore      UMETA(DisplayName="First Score Wins"),
	
	/** 추가 시간을 부여 */
	ExtraTime       UMETA(DisplayName="Extra Time")
};

/**
 * 게임 승리 조건 및 연장전 규칙 정의 구조체
 * - WinType에 따라 승리 판정 방식 결정
 * - 필요 시 목표 값(Target) 및 연장전 규칙 적용
 */
USTRUCT(BlueprintType)
struct FPTWWinConditionRule
{
	GENERATED_BODY()

	/** 승리 조건 유형 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WinCondition")
	EPTWWinType WinType = EPTWWinType::None;

	/** 승리 판정 기준 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WinCondition",
		meta = (EditCondition = "WinType == EPTWWinType::Target || WinType == EPTWWinType::TimeLimit"))
	EPTWWinMetric WinMetric = EPTWWinMetric::None;

	/** 승리 판정 기준 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "WinCondition", meta = (EditCondition = "WinType == EPTWWinType::TimeLimit"))
	float TimeLimitDuration = 90.f;
	
	/** 연장전 규칙 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WinCondition")
	EPTWOvertimeRule OvertimeRule = EPTWOvertimeRule::None;

	/** 연장전 추가 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "WinCondition", meta = (EditCondition = "OvertimeRule == EPTWOvertimeRule::ExtraTime"))
	float OvertimeDuration = 30.f;
};

/**
 * 팀 구성 및 팀 점수 처리 규칙 정의 구조체
 * - 팀전 여부 및 팀 개수 설정
 * - 팀 내 점수 공유 방식 설정
 */
USTRUCT(BlueprintType)
struct FPTWTeamRule
{
	GENERATED_BODY()
	/** 팀전 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Team")
	bool bUseTeam = false;
	
	/** 팀 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Team", meta = (EditCondition = "bUseTeam"))
	int32 NumTeams = 2;
	
	/** 팀 내 점수 공유 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Team", meta = (EditCondition = "bUseTeam"))
	bool bShareScoreWithinTeam = true;
};
/**
 * 데미지 및 피해 처리 규칙 정의 구조체
 * - 자해/낙하 피해 허용 여부
 * - 데미지 배율 및 헤드샷 배율
 * - 원샷 원킬 규칙
 */
USTRUCT(BlueprintType)
struct FPTWDamageRule
{
	GENERATED_BODY()

	/** 자해(Self Damage) 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Damage")
	bool bAllowSelfDamage = false;
	
	/** 낙하 피해(Fall Damage) 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Damage")
	bool bAllowFallDamage = false;
	
	/** 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Damage")
	float DamageMultiplier = 1.0;
	
	/** 헤드샷 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Damage")
	float HeadshotMultiplier = 1.0;
	
	/** 원샷 원킬 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Damage")
	bool bOneShotOneKill = false;
};

/**
 * 랜덤 이벤트 발생 트리거 유형
 * - 이벤트가 언제 발동되는지를 결정하는 기준
 */
UENUM(BlueprintType)
enum class EPTWRandomEventType : uint8
{
	/** 랜덤 이벤트 없음 */
	None	UMETA(DisplayName="None"),

	/** 일정 주기 마다 반복 */
	Interval UMETA(DisplayName = "Interval"),
	
	/** 게임 종료까지 남은 시간이 특정 값 이하일 때 이벤트 발생 */
	TimeRemain UMETA(DisplayName = "TimeRemain"),
	
	/** 생존 인원 수가 특정 임계값 이하가 되었을 때 이벤트 발생 */
	SurvivalThreshold UMETA(DisplayName = "SurvivalThreshold")
};

/**
 * 랜덤 이벤트 발생 및 유지 규칙 정의 구조체
 * - 이벤트 트리거 방식
 * - 적용 지연, 반복 주기, 유지 시간 설정
 */
USTRUCT(BlueprintType)
struct FPTWRandomEventRule
{
	GENERATED_BODY()
	
	/** 랜덤 이벤트 발생 방식 */
	UPROPERTY(EditAnywhere)
	EPTWRandomEventType RandomEventType;
	
	/** 이벤트 적용 전 지연 시간 */
	UPROPERTY(EditAnywhere) 
	float ApplyDelayTime = 5.f;

	/** 이벤트가 반복적으로 발생하는 주기 시간(초) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "RandomEventType==EPTWRandomEventType::Interval")) 
	float IntervalTime = 30.f; 
	
	/** 이벤트 효과가 유지되는 시간(초) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "RandomEventType==EPTWRandomEventType::Interval")) 
	float ApplyDuration = 10.f;
	
	/** 게임 종료까지 남은 시간이 이 값(초) 이하가 되었을 때 이벤트 발생 */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "RandomEventType==EPTWRandomEventType::TimeRemain")) 
	float RemainTimeThreshold = 30.f;
	
	/** 생존 인원 수가 이 값 이하가 되었을 때 이벤트 발생) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "RandomEventType==EPTWRandomEventType::SurvivalThreshold")) 
	int32 MinSurvivorCount = 2; 
};

/**
 * 미니게임 단위로 적용되는 규칙 묶음(RuleSet)
 * - 스폰/장비/전투/이동/승리조건/팀/데미지/랜덤이벤트 등을 하나의 데이터로 관리
 */
USTRUCT(BlueprintType)
struct FPTWMiniGameRule
{
	GENERATED_BODY()
	
	/** 킬/점수/처치 관련 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Kill")
	FPTWKillRule KillRule;
	
	/** 스폰/리스폰 및 스폰 보호 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Spawn")
	FPTWSpawnRule SpawnRule;
	
	/** 기본 무기 지급/드랍 및 무기 제한 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Loadout")
	FPTWLoadoutRule LoadoutRule;

	/** 전투 수단 사용 제한 및 탄약/재장전/스왑 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Combat")
	FPTWCombatRule CombatRule;

	/** 이동 행동 및 이동/중력 스케일, 충돌 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Movement")
	FPTWCombatRule MovementRule;

	/** 승리 조건 및 연장전 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|WinCondition")
	FPTWWinConditionRule WinConditionRule;

	/** 팀전 여부 및 팀 점수 공유 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Team")
	FPTWTeamRule TeamRule;

	/** 데미지 배율/헤드샷/낙하/자해 및 원샷 원킬 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|Damage")
	FPTWDamageRule DamageRule;

	/** 랜덤 이벤트 트리거/지연/유지시간 규칙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule|RandomEvent")
	FPTWRandomEventRule RandomEventRule;

};
