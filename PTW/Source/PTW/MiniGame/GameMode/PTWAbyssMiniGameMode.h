// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWAbyssMiniGameMode.generated.h"

class APlayerState;
class AActor;
class APostProcessVolume;

UCLASS()
class PTW_API APTWAbyssMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbyssMiniGameMode();

protected:
	virtual void StartRound() override;
	virtual void EndRound() override;
	virtual void SpawnDefaultWeapon(AController* NewPlayer) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Abyss|Weapon")
	FGameplayTag AbyssDefaultWeaponTag;
	
	UPROPERTY()
	TObjectPtr<APostProcessVolume> AbyssPP = nullptr;

	void SetAbyssDark(bool bEnable);
	void CacheAbyssPP();

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
