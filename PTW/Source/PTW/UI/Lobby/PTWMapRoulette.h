// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMapRoulette.generated.h"

class UTextBlock;
class UDataTable;
/**
 * 
 */
UCLASS()
class PTW_API UPTWMapRoulette : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 맵 이름 텍스트 상자 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MapNameText;

	/* 맵 정보가 들어있는 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> MiniGameMapTable;

	/* 셔플 속도 (초 단위, 낮을수록 빠름) */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float ShuffleInterval = 0.1f;

private:
	/* 모든 맵 이름을 테이블에서 미리 가져옴 */
	void CacheMapNames();

	/* 룰렛이 도는 동안 랜덤 텍스트 표시 */
	void PlayShuffleAnim();

	/* 최종 결과 표시 */
	void ShowFinalResult();

	FTimerHandle ShuffleTimerHandle;
	TArray<FText> AllMapNames;
};
