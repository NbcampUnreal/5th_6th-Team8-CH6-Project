// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MiniGame/PTWMiniGameRule.h"
#include "PTWGameState.generated.h"

class APTWPlayerState;


#pragma region Enum/Struct

UENUM(BlueprintType)
enum class EPTWGamePhase : uint8
{
	/** 미니 게임 시작 전 로비 */
	PreGameLobby UMETA(DisplayName="Pre Game Lobby"),

	/** 미니 게임 진행 */
	MiniGame UMETA(DisplayName="Mini Game"),

	/** 미니 게임 진행 후 로비 */
	PostGameLobby UMETA(DisplayName="Post Game Lobby")
};

// 룰렛 진행 단계를 나타내는 열거형
UENUM(BlueprintType)
enum class EPTWRoulettePhase : uint8
{
	None            UMETA(DisplayName = "None"),
	MapRoulette   UMETA(DisplayName = "Map Roulette"),
	RoundEventRoulette  UMETA(DisplayName = "Event Roulette"),
	Finished        UMETA(DisplayName = "Finished")
};

USTRUCT(BlueprintType)
struct FPTWRouletteData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EPTWRoulettePhase CurrentPhase;

	UPROPERTY(BlueprintReadOnly)
	FName MapRowName;

	UPROPERTY(BlueprintReadOnly)
	FName EventRowName;

	UPROPERTY(BlueprintReadOnly)
	float RouletteDuration;
};
#pragma endregion

#pragma region Delegate
/**
 * 남은 시간 변경 이벤트
 * - RemainTime이 변경될 때 브로드캐스트
 * - UI/HUD 등에서 구독하여 시간 표시 갱신에 사용
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainTimeChanged, int32, RemainTime);

/**
 * 타이머 종료 이벤트
 * - RemainTime이 0에 도달했을 때 브로드캐스트
 * - 다음 페이즈 전환(예: TravelLevel) 등의 트리거로 사용
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountDownFinished);
/**
 * 게임 페이즈 변경 이벤트
 * - 현재 게임 페이즈가 변경될 때 브로드캐스트
 * - UI 상태 전환, 입력 제한, 룰 적용 타이밍 등에 사용
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EPTWGamePhase, CurrentGamePhase);

/**
 * 라운드 변경 이벤트
 * - 현재 라운드 값이 변경될 때 브로드캐스트
 * - 라운드 UI 갱신, 라운드 시작/종료 연출 트리거 등에 사용
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int32, CurrentRound);

/**
 * 랭킹 플레이어 목록 갱신 이벤트
 * - 랭킹 순서가 변경되었을 때 브로드캐스트
 * - UI 랭킹 표시 갱신에 사용
 *
 * @param RankedPlayers 현재 랭킹 순서대로 정렬된 플레이어 목록
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateRankedPlayers, TArray<APTWPlayerState*>, RankedPlayers);


/**
* 킬로그 방송을 위한 델리게이트
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKilllogBroadcastSignature, AActor*, DeadActor, AActor*, KillerActor);

//원인, 무기 포함 킬로그 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKilllogBroadcastExSignature,AActor*, DeadActor,AActor*, KillerActor,FName, CauseId);
/**
 * 룰렛 단계 변경 이벤트
 * - 룰렛 진행 단계 또는 결과가 변경되었을 때 브로드캐스트
 * - 룰렛 UI 연출 및 상태 전환에 사용
 *
 * @param RouletteData 현재 룰렛 단계 및 선택 결과 데이터
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoulettePhaseChanged, FPTWRouletteData, RouletteData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiniGameRoundChanged, int32, CurrentRound, int32, MaxRound);

/* 채팅 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnChatMessageBroadcast,
	const FString&, Sender,
	const FString&, Message
);

/* 미니게임 카운트다운 시작 여부 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniGameCountdownChanged, bool, bCountdown);
/* 미니게임 카운트다운 숫자 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniGameCountDownValueChanged, int32, CountDown);
/** */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortalCountChanged, int32, Current, int32, Required);

#pragma endregion

UCLASS()
class PTW_API APTWGameState : public AGameState
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	APTWGameState();

	/** 서버에서 호출하여 모든 클라이언트의 델리게이트를 실행시키는 RPC */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastKilllog(AActor* DeadActor, AActor* KillerActor);
	
	//확장한 킬로그 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastKilllogEx(AActor* DeadActor, AActor* KillerActor, FName CauseId);

	/* 채팅 RPC */
	void BroadcastChatMessage(const FString& Sender, const FString& Message);

	/* 미니게임 카운트다운 */
	bool IsMiniGameCountdown() const { return bMiniGameCountdown; }
#pragma region Setter
	/** 남은 시간 설정 */
	void SetRemainTime(int32 NewTime);
	/** 현재 라운드 설정 */
	void SetCurrentRound(int32 NewRound);
	/** 현재 게임 페이즈 설정 */
	void SetCurrentPhase(EPTWGamePhase NewGamePhase);
	/**  */
	void SetRouletteData(const FPTWRouletteData& NewData);

	void SetPortalCount(int32 NewCurrent, int32 NewRequired);

	void SetbMiniGameCountdown(bool bCountdown);
	void SetMiniGameCountdown(int32 NewValue);

	void SetMaxMiniGameRound(int32 NewMaxRound);
#pragma endregion

#pragma region Event
	/** 남은 시간 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnRemainTimeChanged OnRemainTimeChanged;

	/** 타이머 종료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnTimerFinished OnTimerFinished;
	
	/** 카운트 다운 종료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnCountDownFinished OnCountDownFinished;
	
	/** 라운드 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnRoundChanged OnRoundChanged;

	/** 게임 페이즈 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnGamePhaseChanged OnGamePhaseChanged;
	
	/** 랭킹 플레이어 갱신 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnUpdateRankedPlayers OnUpdateRankedPlayers;

	/** 킬로그 이벤트: UI가 이 이벤트를 구독합니다. */
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnKilllogBroadcastSignature OnKilllogBroadcast;
	
	//확장 킬로그 이벤트
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnKilllogBroadcastExSignature OnKilllogBroadcastEx;
	
	/* 채팅 */
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageBroadcast OnChatMessageBroadcast;
	
	/** 룰렛 페이즈 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnRoulettePhaseChanged OnRoulettePhaseChanged;
	
	/** 포탈  */
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnPortalCountChanged OnPortalCountChanged;
	
	/* 미니게임 카운트다운 */
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnMiniGameCountdownChanged OnMiniGameCountdownChanged;
	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnMiniGameCountDownValueChanged OnMiniGameCountdownValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Event")
	FOnMiniGameRoundChanged OnMiniGameRoundChanged;
#pragma endregion

#pragma region Getter
	FORCEINLINE int32 GetRemainTime() const { return RemainTime; }
	FORCEINLINE int32 GetMiniGameCountDown() const { return MiniGameCountDown; }
	FORCEINLINE int32 GetCurrentRound() const {return CurrentRound;}
	FORCEINLINE int32 GetCurrentMiniGameRound() const {return CurrentMiniGameRound;}
	FORCEINLINE int32 GetMaxMiniGameRound() const {return MaxMiniGameRound;}
	FORCEINLINE EPTWGamePhase GetCurrentGamePhase() const {return CurrentGamePhase;}
	FORCEINLINE TArray<APTWPlayerState*> GetRankedPlayers() const {return RankedPlayers;}
	FORCEINLINE FPTWRouletteData GetRouletteData() const {return RouletteData;}
#pragma endregion
	
protected:
	/** 복제 설정 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* 채팅 RPC */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastChatMessage(const FString& Sender, const FString& Message);

	/* 미니게임 카운트다운 */
	UFUNCTION()
	void OnRep_MiniGameCountdown();
	UFUNCTION()
	void OnRep_MiniGameCountDownValue();

	UPROPERTY(ReplicatedUsing = OnRep_MiniGameCountdown)
	bool bMiniGameCountdown = false;
	UPROPERTY(ReplicatedUsing = OnRep_MiniGameCountDownValue)
	int32 MiniGameCountDown = 0;

#pragma region Replication
	/** 남은 시간(초) - 서버에서 갱신, 클라이언트로 복제 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RemainTime, Category = "GameFlow|Timer")
	int32 RemainTime = 0;

	/** RemainTime 복제 갱신 시 호출 */
	UFUNCTION()
	void OnRep_RemainTime();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentRound, Category = "GameFlow|Round")
	int32 CurrentRound =0;

	UFUNCTION()
	void OnRep_CurrentRound();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentGamePhase, Category = "GameFlow|Phase")
	EPTWGamePhase CurrentGamePhase;

	UFUNCTION()
	void OnRep_CurrentGamePhase();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RankedPlayers, Category = "GameFlow|Rank")
	TArray<TObjectPtr<APTWPlayerState>> RankedPlayers;

	UFUNCTION()
	void OnRep_RankedPlayers();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RouletteData, Category = "GameFlow|Roulette")
	FPTWRouletteData RouletteData;

	UFUNCTION()
	void OnRep_RouletteData();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_PortalCount, Category = "Lobby")
	int32 PortalCurrent = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_PortalCount, Category = "Lobby")
	int32 PortalRequired = 0;
	
	UFUNCTION()
	void OnRep_PortalCount();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_CurrentMiniGameRound, Category = "MiniGame|Round")
	int32 CurrentMiniGameRound = 0;

	UFUNCTION()
	void OnRep_CurrentMiniGameRound();
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_MaxMiniGameRound, Category = "MiniGame|Round")
	int32 MaxMiniGameRound = 0;
	
	UFUNCTION()
	void OnRep_MaxMiniGameRound();
#pragma endregion

public:
	/** 서버에서 남은 시간을 감소 */
	void DecreaseTimer();

	/** 카운트 다운 시간을 감소 */
	void DecreaseCoundDown();
	
	/** 라운드 증가 */
	void AdvanceRound();

	/** 미니 게임 라운드 증가 */
	void AdvanceMiniGameRound();
	
	/** 현재 상태 기준으로 랭킹 갱신 */
	void UpdateRanking();
	
	/** 순위 정렬을 위해 플레이어 추가 */
	void AddRankedPlayer(APTWPlayerState* NewPlayerState);
	
	/** 미니 게임 순위를 기준으로 승점 부여 */
	void ApplyMiniGameRankScore(const FPTWMiniGameRule& MiniGameRule);

	/** 생존 플레이어들 */
	UPROPERTY()
	TSet<TObjectPtr<APlayerState>> AlivePlayers;
private:
	/**  */
	
};
