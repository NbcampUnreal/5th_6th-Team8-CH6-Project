// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "PTWRoundEventManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRouletteFinished, FName, OutSelectedMapRowName);


class APTWGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWRoundEventManager : public UActorComponent
{
	GENERATED_BODY()

public:
	//*룰렛 시퀸스 시작 */
	void StartRouletteSequence();

	FString TravelLevelName;
	
	UPROPERTY(BlueprintAssignable)
	FOnRouletteFinished OnRouletteFinished;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void StartRoundEventRoulette();
	UFUNCTION()
	void EndRoulette();
	
private:
	//* 랜덤 맵 선택 */
	void SelectedRandomMap();
	//* 랜덤 이벤트 선택 */
	void SelectedRandomEvent();

	void StartMapRoulette();
	
	//* 조건을 만족하는 모든 맵 RowName 반환 */
	TArray<FName> GetSelectableMapRowNames();
	
	UPROPERTY(EditDefaultsOnly, Category = "Roulette")
	TObjectPtr<UDataTable> MiniGameMapTable;

	UPROPERTY(EditDefaultsOnly, Category = "Roulette")
	TObjectPtr<UDataTable> LobbyRoundEventTable;
	
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;

	FGameplayTag MapTag;
	FGameplayTag EventTag;
	FTimerHandle RouletteTimer;

	int32 RouletteDuration;
};
