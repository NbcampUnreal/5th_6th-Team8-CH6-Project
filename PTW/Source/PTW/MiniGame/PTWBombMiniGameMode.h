// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWBombMiniGameMode.generated.h"

class APTWBombActor;

UCLASS()
class PTW_API APTWBombMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	virtual void OnCountDownFinished() override;

	// 라운드 타이머 종료 시 호출
	virtual void EndTimer() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Weapon Def")
	TObjectPtr<UPTWItemDefinition> BombItemDefinition;

private:
	
	FTimerHandle RoundTimerHandle;
	/** 폭탄 게임 총 라운드 횟수 */
	UPROPERTY(EditDefaultsOnly, Category="Bomb|Round")
	int32 MaxRoundCount = 3;

	/** 현재 라운드  */
	int32 CurrentRound = 0;

	/** 라운드 시작 */
	void StartRound();
	
	UPROPERTY()
	TObjectPtr<APTWPlayerState> BombOwnerPS = nullptr;
	 
	void AssignRandomBombOwner();
	
	void GetAlivePlayerStates(TArray<APTWPlayerState*>& OutAlive) const;
	
	UPROPERTY(EditDefaultsOnly, Category="Bomb")
	TSubclassOf<APTWBombActor> BombActorClass;
	
	UPROPERTY()
	TObjectPtr<APTWBombActor> BombActor = nullptr;
	
	void CleanupBombActor();
};
