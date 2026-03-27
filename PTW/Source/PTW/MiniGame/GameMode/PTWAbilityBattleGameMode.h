// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "PTWAbilityBattleGameMode.generated.h"

class UPTWAbilityBattlePSComponent;
class UAbilitySystemComponent;
class UPTWRandomDraftSystem;
/**
 * 
 */
UCLASS()
class PTW_API APTWAbilityBattleGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

public:
	APTWAbilityBattleGameMode();
	
	virtual void HandleRespawn(APTWPlayerController* PlayerController) override;
protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	virtual void StartGame() override;
	virtual void StartRound() override;
	virtual void RespawnPlayer(APTWPlayerController* SpawnPlayerController) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void StartShieldRegen();
	void StopShieldRegen(AController* DeadPlayer);
	
	void InitAttributeSet();
	
	/** 티어별로 AbilityPool 분류*/
	void InitializeAbilityPool();
	
	/** 랜덤 선택지 생성*/
	TArray<FName> GenerateDraftOptions(int32 Tier);
	
	void StartDraftAllPlayer(int32 Tier);
	void StartDraftChargeTimer();
	void EndDraft();
private:
	void GrandAbilityBattleAttributeSet();
	void AttachPlayerStateComponent(APlayerController* Controller);
	void AddDraftChargeAllPlayers();
	void UpdateChargeTime();

	void ApplyEffect(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> Effect);
	
	TMap<int32, TArray<FName>> TierAbilityPool;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> AbilityDataTable;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> InitAttributeEffectClass;

	//UPROPERTY(EditDefaultsOnly)
	//TSubclassOf<UGameplayEffect> InitRespawnEffectClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWAbilityBattlePSComponent> PSComponentClass; 
		
	int32 DraftOptionCount = 3;

	UPROPERTY(EditDefaultsOnly)
	int32 FirstDraftTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	int32 DraftChargeTime = 25.f;
	
	FTimerHandle FirstDraftTimerHandle;
	FTimerHandle DraftChargeTimerHandle;
};
