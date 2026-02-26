// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MiniGame/PTWMiniGameRule.h"
#include "PTWChaosEventManager.generated.h"

class APTWGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWChaosEventManager : public UActorComponent
{
	GENERATED_BODY()

public:
	void InitChaosEventManager(APTWGameState* InGameState, const FPTWChaosEventRule& Rule);
	void TriggerChaosEvent();

	UFUNCTION()
	void AddChaosItemPool(FName ItemID);
private:
	virtual void BeginPlay() override;
	
	void InitGameState(APTWGameState* InGameState);
	void InitChaosEventRule(const FPTWChaosEventRule& Rule);
	
	FName SelectRandomChaosItem();
	
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;
	
	UPROPERTY()
	TObjectPtr<UDataTable> ChaosItemTable;
	
	FPTWChaosEventRule ChaosEventRule;
	
	//* 플레이어가 구매한 카오스 아이템 보관 */
	TMap<FName, int32> ChaosItemPool;
	
};
