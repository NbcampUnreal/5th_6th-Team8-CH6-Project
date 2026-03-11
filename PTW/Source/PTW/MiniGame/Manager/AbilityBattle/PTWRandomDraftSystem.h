// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWRandomDraftSystem.generated.h"

class APTWGameState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWRandomDraftSystem : public UObject
{
	GENERATED_BODY()

public:
	void InitRandomDraftSystem(APTWGameState* InGameState, UDataTable* InDataTable);
	
	void InitAbilityDataTable(UDataTable* InDataTable);
	void InitGameState(APTWGameState* InGameState);

	// 플레이어가 선택할 수 있는 능력 드래프트 옵션 3개를 랜덤으로 생성한다.
	void GenerateDraftOptions();
private:

	UPROPERTY()
	TObjectPtr<APTWGameState> GameState; 
	
	UPROPERTY()
	TObjectPtr<UDataTable> AbilityDataTable;
	
};
