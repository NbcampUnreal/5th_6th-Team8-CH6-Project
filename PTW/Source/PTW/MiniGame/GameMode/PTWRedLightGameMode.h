// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWRedLightGameMode.generated.h"

class APTWRedLightCharacter;

UCLASS()
class PTW_API APTWRedLightGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	// 움직임 허용 오차 (너무 낮으면 마우스 회전만으로도 죽을 수 있음)
	UPROPERTY(EditDefaultsOnly, Category = "RedLight")
	float MaxAllowedSpeed = 10.0f;

	// 특정 플레이어를 술래로 강제 변경하는 함수
	UFUNCTION(BlueprintCallable, Category = "RedLight")
	void AssignTagger(APlayerController* TaggerPC, TSubclassOf<APTWRedLightCharacter> TaggerClass);

	// 술래 캐릭터가 고개를 돌리거나 원위치할 때 호출됨
	void OnRedLightStateChanged(bool bIsRedLight, APTWRedLightCharacter* TaggerChar);

	// 도망자가 움직여서 마크(Caught)된 상태인지 검증하는 함수 (저격 시 사용)
	bool IsPlayerCaught(ACharacter* PlayerToCheck) const;

protected:
	FTimerHandle MovementCheckTimer;

	// 현재 활성화된 술래 저장
	UPROPERTY()
	APTWRedLightCharacter* CurrentTagger;

	// 이미 발각되어 저격 대기 중인 도망자들 목록
	UPROPERTY()
	TSet<ACharacter*> CaughtPlayers;

	// 주기적으로 도망자들의 속도를 검사하는 함수
	void CheckPlayerMovements();
};
