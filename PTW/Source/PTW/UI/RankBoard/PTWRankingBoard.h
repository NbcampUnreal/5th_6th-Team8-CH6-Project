// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRankingBoard.generated.h"

class UVerticalBox;
class UPTWRankingEntry;
class APTWPlayerState;

/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingBoard : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* PlayerState 델리게이트 바인딩 */
	void BindPlayerStates();
	void UnbindPlayerStates();

	/* 랭킹 갱신 */
	UFUNCTION()
	void UpdateRanking();

	UFUNCTION()
	void OnPlayerDataChanged(const FPTWPlayerData& NewData);

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RankingList;

	UPROPERTY(EditDefaultsOnly, Category = "Ranking")
	TSubclassOf<UPTWRankingEntry> RankingEntryClass;

	/* 캐싱 */
	UPROPERTY()
	TArray<TObjectPtr<APTWPlayerState>> CachedPlayerStates;
};
