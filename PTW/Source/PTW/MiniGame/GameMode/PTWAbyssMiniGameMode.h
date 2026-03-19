// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWAbyssMiniGameMode.generated.h"

class APlayerState;
class AActor;

UCLASS()
class PTW_API APTWAbyssMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbyssMiniGameMode();

protected:
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	virtual void StartRound() override;
	virtual void EndRound() override;
	
	
	UPROPERTY(EditDefaultsOnly, Category="Abyss|Lightning")
	bool bUseLightningFlash = true;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Lightning")
	float LightningFlashDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Lightning")
	float LightningMinInterval = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Abyss|Lightning")
	float LightningMaxInterval = 10.0f;

	FTimerHandle LightningTimerHandle;
	FTimerHandle LightningRestoreTimerHandle;

	void ScheduleLightningFlash();
	void TriggerLightningFlash();
	void RestoreAbyssDark();
	void GiveAndEquipDefaultWeapon(AController* NewPlayer);
	void AbyssRespawnPlayer(APTWPlayerController* SpawnPlayerController);

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

	void TickIdleReveal();
	void ShowReveal(AController* Controller);
	void HideReveal(AController* Controller);
};
