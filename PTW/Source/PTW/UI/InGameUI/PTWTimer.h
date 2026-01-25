// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWTimer.generated.h"

class UTextBlock;
class APTWGameState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWTimer : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 남은 시간 변경 콜백 */
	UFUNCTION()
	void HandleRemainTimeChanged(int32 NewRemainTime);

	/* 시간 포맷 (mm:ss) */
	FText FormatTime(int32 Seconds) const;

	/* 남은 시간 표시 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RemainTimeText;
	/* GameState */
	UPROPERTY()
	APTWGameState* PTWGameState;

};
