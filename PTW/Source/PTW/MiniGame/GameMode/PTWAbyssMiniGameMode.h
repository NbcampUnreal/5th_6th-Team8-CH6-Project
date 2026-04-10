#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWAbyssMiniGameMode.generated.h"

class AActor;
class AController;
class APlayerState;
class APTWPlayerController;

UCLASS()
class PTW_API APTWAbyssMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbyssMiniGameMode();

protected:
	virtual void StartRound() override;
	virtual void EndRound() override;
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	virtual void HandleRespawn(APTWPlayerController* PlayerController) override;

protected:
	// 블랙아웃 사이클 제어 부분
	void StartBlackoutCycle();
	void StopBlackoutCycle();
	void ScheduleNextBlackout();
	void BeginBlackout();
	void FinishBlackout();
	//블랙아웃 상태 적용 부분
	void ApplyBlackoutStateToAllPlayers(bool bEnable);
	void ApplyBlackoutStateToPlayer(APTWPlayerController* PC, bool bEnable);
	void ApplyBlackoutVisual(APTWPlayerController* PC, bool bEnable);
	void ApplyBlackoutStealth(APTWPlayerController* PC, bool bEnable);
	void ApplyBlackoutFireRestriction(APTWPlayerController* PC, bool bEnable);
	//정지 플레이어 Reveal 시스템
	void StartIdleRevealTracking();
	void StopIdleRevealTracking();
	void UpdateIdleReveal();
	void ResetIdleRevealState();
	void ShowReveal(AController* Controller);
	void HideReveal(AController* Controller);
	void ClearAllRevealMarkers();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Blackout")
	bool bUseBlackoutCycle = true;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Blackout")
	float BlackoutDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Blackout")
	float BlackoutMinInterval = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Blackout")
	float BlackoutMaxInterval = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Reveal")
	float IdleRevealTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Reveal")
	float IdleSpeedThreshold = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Reveal")
	float IdleCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Abyss|Reveal")
	TSubclassOf<AActor> RevealMarkerClass;

private:
	FTimerHandle BlackoutTimerHandle;
	FTimerHandle BlackoutEndTimerHandle;
	FTimerHandle IdleRevealTimerHandle;

	TMap<TObjectPtr<APlayerState>, float> IdleTimeMap;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, TObjectPtr<AActor>> RevealMarkerMap;

	bool bIsBlackoutActive = false;
};
