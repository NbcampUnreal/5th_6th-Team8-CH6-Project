#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWAbyssMiniGameMode.generated.h"

class APlayerState;
class AActor;
class AController;

UCLASS()
class PTW_API APTWAbyssMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbyssMiniGameMode();

protected:
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	virtual void StartRound() override;
	virtual void EndRound() override;
	virtual void HandleRespawn(APTWPlayerController* PlayerController) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Abyss|Blackout")
	bool bUseBlackoutCycle = true;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Blackout")
	float BlackoutDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Blackout")
	float BlackoutMinInterval = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Blackout")
	float BlackoutMaxInterval = 15.0f;

	FTimerHandle BlackoutTimerHandle;
	FTimerHandle BlackoutEndTimerHandle;

	void ScheduleBlackout();
	void StartBlackout();
	void EndBlackout();
	void ApplyBlackoutState(bool bEnable);
	void ApplyBlackoutStateToPlayer(APTWPlayerController* PC, bool bEnable);

private:
	UPROPERTY(EditDefaultsOnly, Category="Abyss|Reveal")
	float IdleRevealTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Reveal")
	float IdleSpeedThreshold = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Reveal")
	float IdleCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Reveal")
	TSubclassOf<AActor> RevealMarkerClass;

	FTimerHandle IdleRevealTimerHandle;

	TMap<TObjectPtr<APlayerState>, float> IdleTimeMap;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, TObjectPtr<AActor>> RevealMarkerMap;

	bool bIsBlackoutActive = false;

	void TickIdleReveal();
	void ShowReveal(AController* Controller);
	void HideReveal(AController* Controller);
	void ClearAllRevealMarkers();
};
