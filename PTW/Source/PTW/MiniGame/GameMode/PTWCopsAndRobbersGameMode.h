// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWCopsAndRobbersGameMode.generated.h"

#define ROBBERS 0
#define COPS 1

class UGameplayAbility;
class UGameplayEffect;
class UPTWCARControllerComponent;

UCLASS()
class PTW_API APTWCopsAndRobbersGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWCopsAndRobbersGameMode();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StartGame() override;
	virtual void EndGame() override;
	virtual void WaitingToStartRound() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void AssignTeam() override;
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	
public:
	
protected:
	UPROPERTY(EditAnywhere, Category="GAS|GameplayEffects")
	TSubclassOf<UGameplayEffect> BlindGameplayEffect;
	
	UPROPERTY(EditAnywhere, Category="GAS|GameplayEffects")
	TSubclassOf<UGameplayEffect> ReboundGameplayEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cops|Weapon")
	TObjectPtr<UPTWItemDefinition> CopsWeaponDefinition;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ControllerComponent")
	TSubclassOf<UPTWCARControllerComponent> CARControllerComponentClass;
};
