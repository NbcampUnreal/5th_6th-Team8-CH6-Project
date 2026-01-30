// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/PTWPlayerData.h"
#include "PTWRankingEntry.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 공통 데이터 세팅 (Virtual로 선언하여 자식들이 오버라이드 가능하게 함)
	virtual void SetEntryData(int32 InRank, const FPTWPlayerData& InData, const FPTWPlayerRoundData& InRoundData, FString SteamName, bool bIsMe);

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_Rank;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Text_Name;
};
