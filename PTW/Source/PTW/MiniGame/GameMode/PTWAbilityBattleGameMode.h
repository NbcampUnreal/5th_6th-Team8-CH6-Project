// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "PTWAbilityBattleGameMode.generated.h"

class UPTWRandomDraftSystem;
/**
 * 
 */
UCLASS()
class PTW_API APTWAbilityBattleGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

protected:

	virtual void StartGame() override;
	
	void InitAttributeSet();
	
	/** 티어별로 AbilityPool 분류*/
	void InitializeAbilityPool();
	
	/** 랜덤 선택지 생성*/
	TArray<FName> GenerateDraftOptions(int32 Tier);

	void StartDraft(int32 Tier);

private:
	void GrandAbilityBattleAttributeSet();

	TMap<int32, TArray<FName>> TierAbilityPool;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> AbilityDataTable;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> InitAttributeEffectClass;

	int32 DraftOptionCount = 3;
};
