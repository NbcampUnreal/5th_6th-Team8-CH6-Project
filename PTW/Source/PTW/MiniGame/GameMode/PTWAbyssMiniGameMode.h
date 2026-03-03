// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"
#include "PTWAbyssMiniGameMode.generated.h"

UCLASS()
class PTW_API APTWAbyssMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbyssMiniGameMode();

protected:
	virtual void StartRound() override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void EndRound() override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Abyss|Weapon")
	FGameplayTag AbyssDefaultWeaponTag;
	
	void GiveAbyssDefaultWeapon(AController* NewPlayer);
};
