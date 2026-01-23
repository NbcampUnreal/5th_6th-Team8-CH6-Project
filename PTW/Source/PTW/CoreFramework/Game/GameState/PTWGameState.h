// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PTWGameState.generated.h"

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



/**
 * 게임 진행 상태(타이머 등)를 네트워크로 동기화하는 GameState
 * - 서버에서 RemainTime을 갱신하고 클라이언트로 복제
 * - RepNotify(OnRep_RemainTime)를 통해 변경 사항을 이벤트로 전달
 */
UCLASS()
class PTW_API APTWGameState : public AGameState
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	APTWGameState();

	/** 서버에서 남은 시간을 감소 */
	void DecreaseTimer();

	void AdvanceRound();
	
	void SetRemainTime(int32 NewTime);
	void SetCurrentRound(int32 NewRound);

	int32 GetRemainTime() const { return RemainTime; }

	/** 남은 시간 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnRemainTimeChanged OnRemainTimeChanged;

	/** 타이머 종료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="GameFlow|Event")
	FOnTimerFinished OnTimerFinished;

	FORCEINLINE int32 GetCurrentRound() const {return CurrentRound;}
protected:
	/** 복제 설정 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** 남은 시간(초) - 서버에서 갱신, 클라이언트로 복제 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RemainTime, Category="GameFlow|Timer")
	int32 RemainTime = 0;

	/** RemainTime 복제 갱신 시 호출 */
	UFUNCTION()
	void OnRep_RemainTime();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentRound, Category="GameFlow|Round")
	int32 CurrentRound;

	UFUNCTION()
	void OnRep_CurrentRound();
};
