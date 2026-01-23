// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRankingBoard.generated.h"

class UVerticalBox;
class UPTWRankingEntry;
/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingBoard : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	void UpdateRanking();

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RankingList;

	UPROPERTY(EditDefaultsOnly, Category = "Ranking")
	TSubclassOf<UPTWRankingEntry> RankingEntryClass;
};
